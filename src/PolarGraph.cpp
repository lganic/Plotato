#include <Plotato/PolarGraph.hpp>
#include <Plotato/items/LinePlot.hpp>
#include <Plotato/items/ScatterPlot.hpp>
#include <Plotato/axis/RadiusAxis.hpp>
#include <Plotato/axis/ThetaAxis.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <algorithm>

using namespace plotato;

namespace plotato {
    
void PolarGraph::set_bounds(GraphBounds set_bounds) {

    // First set the auto framing flags
    r_axis_auto_framing = (std::isinf(set_bounds.ymin) || std::isinf(set_bounds.ymax));

    // Then lets set these bounds. If the flags above are false, then these bounds won't be touched, so the user doesn't have to call this again.
    bounds = set_bounds;
}

RadiusAxis* add_radius_axis(AxisStyle style = AxisStyle());
ThetaAxis* add_theta_axis(AxisStyle style = AxisStyle());

RadiusAxis* PolarGraph::add_radius_axis(AxisStyle style) {

    std::lock_guard<std::mutex> lock(axis_mutex);

    auto new_axis = std::make_unique<RadiusAxis>(style);
    RadiusAxis* axis = new_axis.get();

    current_axis_items.emplace_back(std::move(new_axis));

    return axis;
}

ThetaAxis* PolarGraph::add_theta_axis(AxisStyle style) {

    std::lock_guard<std::mutex> lock(axis_mutex);

    auto new_axis = std::make_unique<ThetaAxis>(style);
    ThetaAxis* axis = new_axis.get();

    current_axis_items.emplace_back(std::move(new_axis));

    return axis;
}

double PolarGraph::map_x(double theta, double r, GraphViewport viewport)
{
    // Polar mapping.
    double x = r * std::cos(theta);
    double y = r * std::sin(theta);

    // Map to screen, using bounds convention of the polar graph.
    double t = x / bounds.ymax;

    return viewport.margin_left + (viewport.graph_width / 2) * (1 + t);
}

double PolarGraph::map_y(double theta, double r, GraphViewport viewport)
{
    // Polar mapping.
    double x = r * std::cos(theta);
    double y = r * std::sin(theta);

    // Map to screen, using bounds convention of the polar graph.
    double t = y / bounds.ymax;
    return viewport.margin_top + (viewport.graph_height / 2) * (1 - t);
}

void PolarGraph::draw_to_canvas(cairo_t *cr, uint32_t width, uint32_t height)
{
    if (current_plot_items.size() == 0) {
        // Set the placeholder image, so we know that the graph has been initialized properly, just not set to any data, or plotted. 
        draw_no_data(cr, width, height);

        return;
    }

    // Update bounds before we create the render context object.
    // Check if we need to do some auto framing.
    if (r_axis_auto_framing){
        // Update the bounds first, so we know that everything is up to date before we get started on the draw process.

        GraphBounds auto_bounds;

        for (size_t i = 0; i < current_plot_items.size(); i ++) {
            
            GraphBounds this_object_bounds = current_plot_items[i]->bounds();

            auto_bounds.adjust_with_bounds(this_object_bounds);
        }


        // We only really care about the ymax bounds. Everything else stays the same for a polar graph.
        if(!std::isinf(auto_bounds.ymax)) {
            bounds.ymax = auto_bounds.ymax;
        }

    }

    // Ensure the inarguable polar bounds are respected
    bounds.xmin = 0;
    bounds.xmax = 2 * M_PI; // Radians should be default behind the scenes.
    
    // Create a new viewport / render context which we will pass to the renderers.
    RenderContext rc = get_render_context(cr, width, height, true);

    uint32_t plot_x = rc.current_viewport.margin_left;
    uint32_t plot_y = rc.current_viewport.margin_top;
    uint32_t plot_w = rc.current_viewport.graph_width;
    uint32_t plot_h = rc.current_viewport.graph_height;

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

    // Do the actual plotting.
    {
        std::lock_guard<std::mutex> lock(data_mutex);

        // Loop over all graph elements, and call each of their corresponding draw functions.
        for(int i = 0; i < current_plot_items.size(); i ++){
            current_plot_items[i]->draw(rc);
        }
    }

    draw_axis(rc);
    draw_version_text(cr, width, height);

    {
        std::lock_guard<std::mutex> lock(text_mutex);

        // Loop over all text elements, and call each of their corresponding draw functions.
        for(int i = 0; i < text_items.size(); i ++){
            text_items[i]->draw(rc);
        }
    }

    // Draw the legend. 
    if (style.draw_legend) {
        draw_legend(rc);
    }
}

}