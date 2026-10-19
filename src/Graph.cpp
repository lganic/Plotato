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

double Graph::map_x(double x, double y, GraphViewport viewport) const
{
    double t = (x - bounds.xmin) / (bounds.xmax - bounds.xmin);
    return viewport.margin_left + t * viewport.graph_width;
}

double Graph::map_y(double x, double y, GraphViewport viewport) const
{
    double t = (y - bounds.ymin) / (bounds.ymax - bounds.ymin);
    return viewport.margin_top + viewport.graph_height * (1 - t);
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

    draw_axis(rc);
    draw_version_text(cr, width, height);

    // Draw the legend. 
    if (style.draw_legend) {
        draw_legend(rc);
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