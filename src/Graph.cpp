#include <Plotato/Graph.hpp>
#include <Plotato/items/LinePlot.hpp>
#include <Plotato/items/ScatterPlot.hpp>
#include <Plotato/axis/LinearAxis.hpp>
#include <Plotato/axis/OffsetAxis.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include "version.hpp"
#include "EmbeddedAssets.hpp"

using namespace plotato;

const uint32_t TITLE_PADDING = 10;
const uint32_t INTER_AXIS_PADDING = 5;


namespace plotato {
    
void Graph::set_bounds(GraphBounds set_bounds) {

    // First set the auto framing flags
    x_axis_auto_framing = (std::isinf(set_bounds.xmin) || std::isinf(set_bounds.xmax));
    y_axis_auto_framing = (std::isinf(set_bounds.ymin) || std::isinf(set_bounds.ymax));

    // Then lets set these bounds. If the flags above are false, then these bounds won't be touched, so the user doesn't have to call this again.
    bounds = set_bounds;
}

LinearAxis* Graph::add_linear_axis(AxisSide side, AxisStyle style)
{
    std::lock_guard<std::mutex> lock(axis_mutex);

    auto new_axis = std::make_unique<LinearAxis>(side, style);
    LinearAxis* axis = new_axis.get();

    current_axis_items.emplace_back(std::move(new_axis));

    return axis;
}

OffsetAxis* Graph::add_offset_axis(AxisSide side, AxisStyle style)
{
    std::lock_guard<std::mutex> lock(axis_mutex);

    auto new_axis = std::make_unique<OffsetAxis>(side, style);
    OffsetAxis* axis = new_axis.get();

    current_axis_items.emplace_back(std::move(new_axis));

    return axis;
}

double Graph::map_x(double x, uint32_t plot_x, uint32_t plot_w) const
{
    double t = (x - bounds.xmin) / (bounds.xmax - bounds.xmin);
    return plot_x + t * plot_w;
}

double Graph::map_y(double y, uint32_t plot_y, uint32_t plot_h) const
{
    double t = (y - bounds.ymin) / (bounds.ymax - bounds.ymin);
    return plot_y + plot_h - t * plot_h;
}

void Graph::draw_to_canvas(cairo_t *cr, uint32_t width, uint32_t height)
{
    // TODO: I don't like how many things are in this draw function. Cleanup is required.

    std::lock_guard<std::mutex> draw_guard(draw_mutex); // Ensure that only one draw is being called at a time.

    if (current_plot_items.size() == 0) {
        // Set the placeholder image, so we know that the graph has been initialized properly, just not set to any data, or plotted. 
        draw_no_data(cr, width, height);

        return;
    }

    // Set background color on the plot.
    style.background_color.to_cairo_source(cr);
    cairo_paint(cr); // Paint the background.

    // Set default margins from style.
    int32_t left_margin = style.default_margin;
    int32_t right_margin = style.default_margin;
    int32_t top_margin = style.default_margin;
    int32_t bottom_margin = style.default_margin;

    if (!style.dont_draw_version_text) bottom_margin += 10;

    // Adjust the margins based on the axis which are present
    {
        std::lock_guard<std::mutex> lock(axis_mutex);

        for (size_t i = 0; i < current_axis_items.size(); i ++) {
            AxisPixelSize axis_size = current_axis_items[i]->size();

            switch (current_axis_items[i]->side)
            {
            case TOP:
                top_margin += axis_size.top;
                top_margin += INTER_AXIS_PADDING;
                break;

            case BOTTOM:
                bottom_margin += axis_size.bottom;
                bottom_margin += INTER_AXIS_PADDING;
                break;

            case LEFT:
                left_margin += axis_size.left;
                left_margin += INTER_AXIS_PADDING;
                break;
            
            case RIGHT:
                right_margin += axis_size.right;
                right_margin += INTER_AXIS_PADDING;
                break;
            
            default:
                break;
            }
        }
    }

    // Get some plot information based on the margin.
    int32_t plot_x = left_margin;
    int32_t plot_y = top_margin;
    int32_t plot_w = width - left_margin - right_margin;
    int32_t plot_h = height - top_margin - bottom_margin;

    if (plot_w <= 0 || plot_h <= 0) {

        // Plot too small! Fill the area with red, to indicate a problem.

        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_paint(cr);

        return; // Do not proceed.
    }

    // Check if we need to do some auto framing.
    if (x_axis_auto_framing || y_axis_auto_framing){
        // Update the bounds first, so we know that everything is up to date before we get started on the draw process.

        GraphBounds auto_bounds;

        for (size_t i = 0; i < current_plot_items.size(); i ++) {
            
            GraphBounds this_object_bounds = current_plot_items[i]->bounds();

            auto_bounds.adjust_with_bounds(this_object_bounds);
        }

        // Save values to the bounds object.
        if (x_axis_auto_framing) {
            if (!std::isinf(auto_bounds.xmin)){
                bounds.xmin = auto_bounds.xmin;
            }
            if(!std::isinf(auto_bounds.xmax)) {
                bounds.xmax = auto_bounds.xmax;
            }
        }
        if (y_axis_auto_framing) {
            if (!std::isinf(auto_bounds.ymin)){
                bounds.ymin = auto_bounds.ymin;
            }
            if (!std::isinf(auto_bounds.ymax)) {
                bounds.ymax = auto_bounds.ymax;
            }
        }
        
    }

    // Plot background
    style.plot_background_color.to_cairo_source(cr);
    cairo_rectangle(cr, plot_x, plot_y, plot_w, plot_h);
    cairo_fill(cr);

    if (style.draw_border) {
        // Draw the border around the graph area. First start by setting the color.
        style.border_color.to_cairo_source(cr);
        cairo_set_line_width(cr, 1.0);
    
        cairo_move_to(cr, plot_x, plot_y); // Move to the top left of the graph area.
        cairo_line_to(cr, plot_x, plot_y + plot_h); // Line on the left hand side.
        cairo_line_to(cr, plot_x + plot_w, plot_y + plot_h); // Line on the bottom.
        cairo_line_to(cr, plot_x + plot_w, plot_y); // Line on the right hand side.
        cairo_line_to(cr, plot_x, plot_y); // Line on the top.
        cairo_stroke(cr); // Stroke the border.
    }

    // Do the actual plotting.
    // Create a new viewport / render context which we will pass to the renderers.
    GraphViewport gv;
    
    gv.bounds = bounds;
    
    gv.graph_height = plot_h;
    gv.graph_width = plot_w;

    gv.margin_left = left_margin;
    gv.margin_right = right_margin;
    gv.margin_top = top_margin;
    gv.margin_bottom = bottom_margin;
    
    RenderContext rc(cr, gv);

    {
        std::lock_guard<std::mutex> lock(data_mutex);

        // Loop over all graph elements, and call each of their corresponding draw functions.
        for(int i = 0; i < current_plot_items.size(); i ++){
            current_plot_items[i]->draw(rc);
        }
    }

    {
        std::lock_guard<std::mutex> lock(axis_mutex);

        int32_t left_axis_offset = 0;
        int32_t right_axis_offset = 0;
        int32_t top_axis_offset = 0;
        int32_t bottom_axis_offset = 0;
    
        // Loop over all the axis, and render each to the graph.
        // This has to use a switch case, in order to correctly stack multi axis plots.
        for(int i = 0; i < current_axis_items.size(); i ++){
    
            AxisPixelSize size = current_axis_items[i]->size();
    
            switch (current_axis_items[i]->side)
            {
            case TOP:
                current_axis_items[i] -> draw(rc, 0, -top_axis_offset);
                top_axis_offset += size.top + INTER_AXIS_PADDING;
                break;
    
            case BOTTOM:
                current_axis_items[i] -> draw(rc, 0, bottom_axis_offset);
                bottom_axis_offset += size.bottom + INTER_AXIS_PADDING;
                break;
    
            case LEFT:
                current_axis_items[i] -> draw(rc, -left_axis_offset, 0);
                left_axis_offset += size.left + INTER_AXIS_PADDING;
                break;
    
            case RIGHT:
                current_axis_items[i] -> draw(rc, right_axis_offset, 0);
                right_axis_offset += size.right + INTER_AXIS_PADDING;
                break;
            }
        }
    }

    draw_version_text(cr, width, height);

    // Draw the legend. 
    if (style.draw_legend) {

        // TODO: Allow the user to place the legend wherever, instead of just in the top right corner.

        // Lets calculate the total size of the thing. 
        // To make this look good, we are going to need to know the size of all the text elements ahead of time.
        // So lets make a vector of all the text extents

        std::vector<std::unique_ptr<cairo_text_extents_t>> all_extents;

        // Set the text style, so we have up to date font info.
        style.legend_text_style.to_cairo_source(cr);

        // Do this inside of a scope, since we are accessing mutexed data.
        {
            std::lock_guard<std::mutex> plot_data_mutex(data_mutex);

            for (size_t plot_item_index = 0; plot_item_index < current_plot_items.size(); plot_item_index ++) {

                if (current_plot_items[plot_item_index]->check_is(PlotType::LINEPLOT)) {

                    LinePlot* line_plot = static_cast<LinePlot*>(current_plot_items[plot_item_index].get());

                    // Make a new extents object.
                    auto new_extents = std::make_unique<cairo_text_extents_t>();
    
                    // Update the cairo text extents.
                    cairo_text_extents(cr, line_plot->style.name.c_str(), new_extents.get());
    
                    // Add the text extents ptr to the array.
                    all_extents.emplace_back(std::move(new_extents));
                }

            }
        }

        // Now we can calculate the text size. 
        // I am going to use the maximum of the height, instead of each element so spacing is even.
        double max_height = 0;
        double max_width = 0;

        for (size_t extents_index = 0; extents_index < all_extents.size(); extents_index ++) {
            max_height = std::max(max_height, all_extents[extents_index]->height);
            max_width = std::max(max_width, all_extents[extents_index]->width);
        }

        // Final size calculation.
        double legend_box_width = max_width + 2 * style.legend_padding + max_height + 5;
        double legend_box_height = all_extents.size() * (max_height + style.legend_inter_object_padding) - style.legend_inter_object_padding + 2 * style.legend_padding;

        // Calculate the legend based on the anchor position.
        auto [plot_off_x, plot_off_y] = get_offset(plot_w, plot_h, style.legend_anchor);
        auto [anchor_x, anchor_y] = get_offset(legend_box_width, legend_box_height, style.legend_anchor);
        auto [padding_x, padding_y] = get_offset(2 * style.legend_offset, 2 * style.legend_offset, style.legend_anchor); // Hacky.

        // First, calculate a legend position which puts it in the exact center of the graph.
        double legend_x = plot_x + (plot_w / 2) - (legend_box_width / 2);
        double legend_y = plot_y + (plot_h / 2) - (legend_box_height / 2);

        // Then use the anchors to offset into the correct position.
        legend_x -= plot_off_x - anchor_x - padding_x;
        legend_y -= plot_off_y - anchor_y - padding_y;

        double swatch_x = legend_x + style.legend_padding;
        double text_x = swatch_x + max_height + 5;

        // Draw legend filled rectangle
        style.legend_color.to_cairo_source(cr);
        cairo_rectangle(cr, legend_x, legend_y, legend_box_width, legend_box_height);
        cairo_fill_preserve(cr); // Fill, and keep path

        style.legend_border_color.to_cairo_source(cr);
        cairo_set_line_width(cr, style.legend_border_width);
        cairo_stroke(cr); // Use our kept path from the rectangle to stroke

        // Now we can draw the text and the color swatches.
        // Do this inside of a scope, since we are accessing mutexed data.
        style.legend_text_style.to_cairo_source(cr);
        {
            std::lock_guard<std::mutex> plot_data_mutex(data_mutex);

            for (size_t plot_item_index = 0; plot_item_index < current_plot_items.size(); plot_item_index ++) {

                if (current_plot_items[plot_item_index]->check_is(PlotType::LINEPLOT)) {

                    LinePlot* line_plot = static_cast<LinePlot*>(current_plot_items[plot_item_index].get());

                    style.legend_text_style.to_cairo_source(cr);
    
                    cairo_move_to(cr, text_x, max_height + legend_y + style.legend_offset + plot_item_index * (max_height + style.legend_inter_object_padding));
    
                    cairo_show_text(cr, line_plot->style.name.c_str());
    
                    // Draw the swatch color
                    line_plot->style.line_color.to_cairo_source(cr);
                    cairo_rectangle(cr, legend_x + style.legend_offset, legend_y + style.legend_offset + plot_item_index * (max_height + style.legend_inter_object_padding), max_height, max_height);
                    cairo_fill_preserve(cr); // Fill, and keep path
    
                    style.legend_border_color.to_cairo_source(cr);
                    cairo_set_line_width(cr, style.legend_border_width);
                    cairo_stroke(cr); // Use our kept path from the rectangle to stroke
                }
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(text_mutex);

        // Loop over all text elements, and call each of their corresponding draw functions.
        for(int i = 0; i < text_items.size(); i ++){
            text_items[i]->draw(rc);
        }
    }
}

}