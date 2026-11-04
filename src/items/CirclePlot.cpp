#include <vector>
#include <algorithm>
#include <Plotato/items/CirclePlot.hpp>
#include <Plotato/util/GraphRenderer.hpp>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/StyleStructs.hpp>

namespace plotato {

CirclePlot::CirclePlot(double x, double y, double radius, ShapeStyle shape_style): x(x), y(y), radius(radius), style(shape_style){}

void CirclePlot::draw(RenderContext& ctx) {
    // We can just pass straight to the renderers circle drawer
    ctx.renderer.draw_circle(x, y, radius, style, margin);
}

GraphBounds CirclePlot::bounds(){
    return GraphBounds{
        x - radius,
        x + radius,
        y - radius,
        y + radius
    };
}

void CirclePlot::draw_legend_element(RenderContext& rc, double x, double y, double width, double height, Color border_color, double border_width) {

    double pixel_radius = std::min(width, height) / 2;

    ShapeStyle legend_item_style;
    legend_item_style.fill = style.fill;
    legend_item_style.outline = border_color;
    legend_item_style.outline_width = border_width;

    rc.renderer.draw_circle_absolute(x + pixel_radius, y + pixel_radius, pixel_radius, legend_item_style);
}

}