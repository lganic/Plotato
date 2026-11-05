#include <vector>
#include <algorithm>
#include <Plotato/items/ScatterPlot.hpp>
#include <Plotato/util/GraphRenderer.hpp>
#include <Plotato/util/GraphBounds.hpp>
#include <iostream>

namespace plotato {

ScatterPlot::ScatterPlot(std::vector<double> x_data, std::vector<double> y_data, MarkerStyle plot_style): x(x_data), y(y_data), style(plot_style){}
ScatterPlot::ScatterPlot(std::vector<double> y_data, MarkerStyle plot_style): y(y_data), style(plot_style) {
    contains_x_data = false;
}

void ScatterPlot::draw(RenderContext& ctx) {

    if (contains_x_data) {
        // Loop over all the x data, and plot each individual element using the graphrenderer.  
        for (size_t i = 0; i < x.size(); i ++) {
            ctx.renderer.draw_marker(x[i], y[i], style);
        }
    }
    else {
        // No X data. use the index instead.

        for (size_t i = 0; i < y.size(); i ++) {
            ctx.renderer.draw_marker(static_cast<double>(i), y[i], style);
        }
    }
}

GraphBounds ScatterPlot::bounds(){
    auto y_result = std::minmax_element(y.begin(), y.end());

    if (contains_x_data) {
        // Find both min and max elements of the x and y array.
        auto x_result = std::minmax_element(x.begin(), x.end());

        // Return the bounds object
        return GraphBounds{*x_result.first, *x_result.second, *y_result.first, *y_result.second};
    }

    // Find calculate x min and max are just 0 and the length of the y array -1
    return GraphBounds{0, static_cast<double>(y.size()) - 1, *y_result.first, *y_result.second};
}

void ScatterPlot::draw_legend_element(RenderContext& rc, double x, double y, double width, double height, Color border_color, double border_width) {

    // Make a new marker style, which we will adjust in order to fit with the legend styling

    MarkerStyle legend_marker_style(style);

    double marker_radius = std::min(width / 2, height / 2);

    legend_marker_style.size = marker_radius;
    legend_marker_style.outline = border_color;
    legend_marker_style.outline_width = border_width;

    rc.renderer.draw_marker_absolute(x + marker_radius, y + marker_radius, legend_marker_style);
}

}