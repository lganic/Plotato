#include <vector>
#include <algorithm>
#include <Plotato/items/PolygonPlot.hpp>
#include <Plotato/util/GraphRenderer.hpp>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/StyleStructs.hpp>

namespace plotato {

PolygonPlot::PolygonPlot(std::vector<double> x_data, std::vector<double> y_data, ShapeStyle shape_style): XYPlot(x_data, y_data), style(shape_style){}

void PolygonPlot::draw(RenderContext& ctx) {

    // We can just pass straight to the renderers polygon drawer
    ctx.renderer.draw_polygon(x, y, style);

}

void PolygonPlot::draw_legend_element(RenderContext& rc, double x, double y, double width, double height, Color border_color, double border_width) {

    std::vector<double> x_poly{
        x + 0.2 * width,
        x + 0.7 * width,
        x + 1.0 * width,
        x + 0.7 * width,
        x + 0.5 * width,
        x + 0.0 * width
    };

    std::vector<double> y_poly{
        y + 0.2 * height,
        y + 0.0 * height,
        y + 0.5 * height,
        y + 1.0 * height,
        y + 0.5 * height,
        y + 0.6 * height
    };

    ShapeStyle legend_item_style;
    legend_item_style.fill = style.fill;
    legend_item_style.outline = border_color;
    legend_item_style.outline_width = border_width;


    rc.renderer.draw_polygon_absolute(x_poly, y_poly, legend_item_style);
}

}