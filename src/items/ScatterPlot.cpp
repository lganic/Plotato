#include <vector>
#include <algorithm>
#include <Plotato/items/ScatterPlot.hpp>
#include <Plotato/util/GraphRenderer.hpp>
#include <Plotato/util/GraphBounds.hpp>
#include <iostream>

namespace plotato {

ScatterPlot::ScatterPlot(std::vector<double> x_data, std::vector<double> y_data, PlotStyle plot_style): x(x_data), y(y_data){
    style = plot_style;
}

void ScatterPlot::draw(RenderContext& ctx) {

    // First, create a graph renderer, which will abstract away a lot of the drawing logic.
    GraphRenderer renderer(ctx.cr, ctx.current_viewport);

    // Loop over all the x data, and plot each individual element using the graphrenderer.  

    for (size_t i = 0; i < x.size(); i ++) {
        renderer.draw_marker(x[i], y[i], style.marker);
    }
}

GraphBounds ScatterPlot::bounds(){

    // I am going to assume that x, and y are of the same size. TODO: Check.

    // Find both min and max elements of the x and y array.
    auto x_result = std::minmax_element(x.begin(), x.end());
    auto y_result = std::minmax_element(y.begin(), y.end());

    // Return the bounds object
    return GraphBounds{*x_result.first, *x_result.second, *y_result.first, *y_result.second};
}

}