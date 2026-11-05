#include <vector>
#include <algorithm>
#include <Plotato/items/LinePlot.hpp>
#include <Plotato/util/GraphRenderer.hpp>
#include <Plotato/util/GraphBounds.hpp>

namespace plotato {

LinePlot::LinePlot(std::vector<double> x_data, std::vector<double> y_data, PlotStyle plot_style): XYPlot(x_data, y_data), style(plot_style){}
LinePlot::LinePlot(std::vector<double> y_data, PlotStyle plot_style): XYPlot(y_data), style(plot_style){}

void LinePlot::draw(RenderContext& ctx) {
    if (contains_x_data) {

        // We have independent X data. Pass directly to graph renderer.
        // Pass our line data to the graph renderer. 
        ctx.renderer.draw_polyline(x, y, style);

        if (marker_active) {
            
            // Loop over all the x data, and plot each individual element using the graphrenderer.  
            for (size_t i = 0; i < x.size(); i ++) {
                ctx.renderer.draw_marker(x[i], y[i], style.marker);
            }

        }
    }
    else {
        // We only have the y data. Use the y data specific drawing methods of the renderer

        ctx.renderer.draw_polyline(y, style);

        if (marker_active) {
            for (size_t i = 0; i < y.size(); i ++) {
                ctx.renderer.draw_marker(static_cast<double>(i), y[i], style.marker);
            }
        }
    }
}

void LinePlot::marker(MarkerStyle marker_style) {

    marker_active = true;
    style.marker = marker_style;

}

void LinePlot::draw_legend_element(RenderContext& rc, double x, double y, double width, double height, Color border_color, double border_width) {
    // Draw the swatch color
    style.line_color.to_cairo_source(rc.cr);
    cairo_rectangle(rc.cr, x, y, width, height);
    cairo_fill_preserve(rc.cr); // Fill, and keep path

    border_color.to_cairo_source(rc.cr);
    cairo_set_line_width(rc.cr, border_width);
    cairo_stroke(rc.cr); // Use our kept path from the rectangle to stroke
}

}