#include <Plotato/util/GraphRenderer.hpp>
#include <cmath>

namespace plotato
{

GraphRenderer::GraphRenderer(cairo_t *cr, const GraphViewport &viewport)
    : cr(cr), viewport(viewport) // Default constructors for this, to optimize initialization pathways.
{

    // Assign the values of the plot rect here.
    plot_rect.x = viewport.margin_left;
    plot_rect.y = viewport.margin_top;
    plot_rect.width = viewport.graph_width;
    plot_rect.height = viewport.graph_height;
}

double GraphRenderer::data_to_screen_x(double x) const
{
    double t = (x - viewport.xmin) / (viewport.xmax - viewport.xmin);
    return plot_rect.x + t * plot_rect.width;
}

double GraphRenderer::data_to_screen_y(double y) const
{
    double t = (y - viewport.ymin) / (viewport.ymax - viewport.ymin);
    return plot_rect.y + plot_rect.height - t * plot_rect.height;
}

void GraphRenderer::set_color(const Color& color){
    cairo_set_source_rgba(cr, color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0);
}

void GraphRenderer::draw_polyline(const std::vector<double>& x,
                                  const std::vector<double>& y,
                                  const PlotStyle& style)
{
    size_t n = std::min(x.size(), y.size());

    if (n < 2) return; // Not enough points! Abort!

    cairo_save(cr); // Save the current graphics paint settings, so that we don't mess anything up that something else has going on.

    // Set the current painting settings to those indicated by the line style.
    set_color(style.line_color);
    cairo_set_line_width(cr, style.line_width);

    bool started = false;

    for (size_t i = 0; i < n; i++) {

        if (!std::isfinite(x[i]) || !std::isfinite(y[i])) {

            // Skip over a coordinate that had invalid data. Indicate that the line is no longer started, so we don't bridge data which wasn't actually next to eachother.
            started = false;
            continue;
        }

        // Map the coordinate to the screen space.
        double sx = data_to_screen_x(x[i]);
        double sy = data_to_screen_y(y[i]);

        if (!started) {
            // Line segment not started yet. Just move to the coordinate. We will draw the line in a sec.
            cairo_move_to(cr, sx, sy);
            started = true;
        } else {
            // Line segment started. So we are currently at the previous point on the line. Stroke to the next point.
            cairo_line_to(cr, sx, sy);
        }
    }

    cairo_stroke(cr); // Stroke that
    cairo_restore(cr); // Restore the graphics paint settings to what we saved them to previously.
}

}