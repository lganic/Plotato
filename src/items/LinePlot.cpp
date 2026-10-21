#include <vector>
#include <algorithm>
#include <Plotato/items/LinePlot.hpp>
#include <Plotato/util/GraphRenderer.hpp>
#include <Plotato/util/GraphBounds.hpp>

namespace plotato {

LinePlot::LinePlot(std::vector<double> x_data, std::vector<double> y_data, PlotStyle plot_style): x(x_data), y(y_data){
    style = plot_style;
}

void LinePlot::draw(RenderContext& ctx) {
    // Pass our line data to the graph renderer. 
    ctx.renderer.draw_polyline(x, y, style);

    if (marker_active) {
        
        // Loop over all the x data, and plot each individual element using the graphrenderer.  
        for (size_t i = 0; i < x.size(); i ++) {
            ctx.renderer.draw_marker(x[i], y[i], style.marker);
        }

    }
}

GraphBounds LinePlot::bounds(){

    // I am going to assume that x, and y are of the same size. TODO: Check.

    // Find both min and max elements of the x and y array.
    auto x_result = std::minmax_element(x.begin(), x.end());
    auto y_result = std::minmax_element(y.begin(), y.end());

    // Return the bounds object
    return GraphBounds{*x_result.first, *x_result.second, *y_result.first, *y_result.second};
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