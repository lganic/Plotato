#include <vector>
#include <algorithm>
#include <Plotato/items/PolygonPlot.hpp>
#include <Plotato/util/GraphRenderer.hpp>
#include <Plotato/util/GraphBounds.hpp>

namespace plotato {

PolygonPlot::PolygonPlot(std::vector<double> x_data, std::vector<double> y_data, ShapeStyle shape_style): x(x_data), y(y_data), style(shape_style){}

void PolygonPlot::draw(RenderContext& ctx) {

    // We can just pass straight to the renderers polygon drawer
    ctx.renderer.draw_polygon(x, y, style);

}

GraphBounds PolygonPlot::bounds(){

    // I am going to assume that x, and y are of the same size. TODO: Check.

    // Find both min and max elements of the x and y array.
    auto x_result = std::minmax_element(x.begin(), x.end());
    auto y_result = std::minmax_element(y.begin(), y.end());

    // Return the bounds object
    return GraphBounds{*x_result.first, *x_result.second, *y_result.first, *y_result.second};
}

void PolygonPlot::draw_legend_element(RenderContext& rc, double x, double y, double width, double height, Color border_color, double border_width) {
    // TODO
}

}