#include <vector>
#include <algorithm>
#include <Plotato/items/LinePlot.hpp>
#include <Plotato/util/GraphRenderer.hpp>
#include <Plotato/util/GraphBounds.hpp>

namespace plotato {

LinePlot::LinePlot(std::vector<double> x_data, std::vector<double> y_data): x(x_data), y(y_data){}

void LinePlot::draw(RenderContext& ctx) {

    // First, create a graph renderer, which will abstract away a lot of the drawing logic.

    GraphRenderer renderer(ctx.cr, ctx.current_viewport);

    // TODO: Don't do this. Have the user able to control this.
    LineStyle ls;

    // Pass our line data to the graph renderer. 
    renderer.draw_polyline(x, y, ls);
}

GraphBounds LinePlot::bounds(){

    // I am going to assume that x, and y are of the same size. TODO: Check.

    // Find both min and max elements of the x and y array.
    auto x_result = std::minmax_element(x.begin(), x.end());
    auto y_result = std::minmax_element(y.begin(), y.end());

    // Return the bounds object
    return GraphBounds{*x_result.first, *x_result.second, *y_result.first, *y_result.second};
}

}