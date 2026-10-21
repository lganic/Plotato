#include <Plotato/axis/RadiusAxis.hpp>
#include <Plotato/util/RenderContext.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <gtk/gtk.h>
#include <cstdint>
#include <cmath>

namespace plotato {

void RadiusAxis::draw(RenderContext& ctx, int32_t offset_x, int32_t offset_y) {

    if (style.num_ticks <= 0) {
        style.num_ticks = -std::round((ctx.current_viewport.graph_width / 2 + 4 * get_approx_vertical_size(style.font_size)) / (get_approx_vertical_size(style.font_size) * (4 + 1)));
    }

    int num_ticks = abs(style.num_ticks);

    // Find the exact pixel center of the circle. We can use the graph renderer for this, and just map the cord 0,0
    double center_x = ctx.renderer.data_to_screen_x(0, 0);
    double center_y = ctx.renderer.data_to_screen_y(0, 0);

    // Now we just need to figure out the pixel radius. Luckily this is actually the easy part, since we can just divide the graph width in half.
    double outside_radius = ctx.current_viewport.graph_width / 2;

    for (int i = 1; i < num_ticks; i ++){

        double radius = outside_radius * i / (num_ticks - 1);

        ctx.renderer.draw_pixel_circle(center_x, center_y, radius, Color(0, 0, 0, 0), style.plot_line_color);
    }
}

}