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

using namespace plotato;

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

double Graph::map_x(double x, double y, GraphViewport viewport)
{
    double t = (x - bounds.xmin) / (bounds.xmax - bounds.xmin);
    return viewport.margin_left + t * viewport.graph_width;
}

double Graph::map_y(double x, double y, GraphViewport viewport)
{
    double t = (y - bounds.ymin) / (bounds.ymax - bounds.ymin);
    return viewport.margin_top + viewport.graph_height * (1 - t);
}

void Graph::draw_to_canvas(cairo_t *cr, uint32_t width, uint32_t height)
{
    if (current_plot_items.size() == 0) {
        // Set the placeholder image, so we know that the graph has been initialized properly, just not set to any data, or plotted. 
        draw_no_data(cr, width, height);

        return;
    }

    // Update bounds before we create the render context object.
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

    // Create a new viewport / render context which we will pass to the renderers.
    RenderContext rc = get_render_context(cr, width, height);

    int32_t plot_x = rc.current_viewport.margin_left;
    int32_t plot_y = rc.current_viewport.margin_top;
    int32_t plot_w = rc.current_viewport.graph_width;
    int32_t plot_h = rc.current_viewport.graph_height;

    // Set background color on the plot.
    style.background_color.to_cairo_source(cr);
    cairo_paint(cr); // Paint the background.

    if (plot_w <= 0 || plot_h <= 0) {

        // Plot too small! Fill the area with red, to indicate a problem.

        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_paint(cr);

        return; // Do not proceed.
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

    draw_items(rc);
    draw_axis(rc);
    draw_version_text(cr, width, height);
    draw_text_items(rc);
    draw_legend(rc);
}

}