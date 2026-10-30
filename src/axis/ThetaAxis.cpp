#include <Plotato/axis/ThetaAxis.hpp>
#include <Plotato/util/RenderContext.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <gtk/gtk.h>
#include <cstdint>
#include <cmath>

namespace plotato {

void ThetaAxis::draw(RenderContext& ctx, int32_t offset_x, int32_t offset_y) {

    if (style.num_ticks <= 0) {
        style.num_ticks = -std::round((ctx.current_viewport.graph_width / 2 + 4 * get_approx_vertical_size(style.font_size)) / (get_approx_vertical_size(style.font_size) * (4 + 1)));
    }

    uint32_t num_ticks = abs(style.num_ticks);

    // Find the exact pixel center of the circle. We can use the graph renderer for this, and just map the cord 0,0
    double center_x = ctx.renderer.data_to_screen_x(0, 0);
    double center_y = ctx.renderer.data_to_screen_y(0, 0);

    // Now we just need to figure out the pixel radius. Luckily this is actually the easy part, since we can just divide the graph width in half.
    double outside_radius = ctx.current_viewport.graph_width / 2;


    style.plot_line_color.to_cairo_source(ctx.cr);

    for (uint32_t i = 0; i < num_ticks - 1; i ++){

        double theta = 2 * M_PI * i / (num_ticks - 1);

        cairo_move_to(ctx.cr, center_x, center_y);

        // Flip angular sign in rendering, because Y is down in viewport. This is so when I add the text later, the theta variable is accurate to where the line actually is.
        cairo_line_to(ctx.cr, center_x + outside_radius * std::cos(-theta), center_y + outside_radius * std::sin(-theta));
        cairo_stroke(ctx.cr);
    }
}

}