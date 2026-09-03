#include <Plotato/util/GraphRenderer.hpp>
#include <Plotato/util/Anchor.hpp>
#include <cmath>
#include <vector>
#include <iostream>

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

void GraphRenderer::draw_interleaved_polygon(double x, double y, uint32_t n, std::vector<double> angles_degrees, std::vector<double> scales, MarkerStyle& style) {

    cairo_save(cr); // Save the current graphics paint settings, so that we don't mess anything up that something else has going on.

    if (angles_degrees.size() != scales.size()) return;

    if (n * angles_degrees.size() <= 2) return; // Shouldn't do anything.

    std::vector<double> angles_radians;
    std::vector<double> scaled_sizes;
    angles_radians.resize(angles_degrees.size());
    scaled_sizes.resize(scales.size());
    
    // First, cast all the angles to radians, and scale the sizes to the scale given in the style struct.
    for(size_t i = 0; i < angles_degrees.size(); i ++) {
        angles_radians[i] = angles_degrees[i] * M_PI / 180.0;
        scaled_sizes[i] = scales[i] * style.size;
    }

    // Map the polygon coordinates to the screen space.
    double poly_x = data_to_screen_x(x);
    double poly_y = data_to_screen_y(y);

    bool started = false;

    for(uint32_t i_vertex = 0; i_vertex < n; i_vertex++)
    {
        double angle_offset = static_cast<double>(i_vertex) * 2.0 * M_PI / static_cast<double>(n);

        for (size_t polygon_index = 0; polygon_index < scaled_sizes.size(); polygon_index ++){
            double sx = poly_x + scaled_sizes[polygon_index] * std::cos(angles_radians[polygon_index] + angle_offset);
            double sy = poly_y + scaled_sizes[polygon_index] * std::sin(angles_radians[polygon_index] + angle_offset);

            if (!started) {
                // Move to the start of the polygon.
                cairo_move_to(cr, sx, sy);
            }
            else
            {
                // Move the path to the next segment of the polygon.
                cairo_line_to(cr, sx, sy);
            }

            started = true;
        }
    }

    // Path complete. Close it.
    cairo_close_path(cr);

    // Now we can fill it
    style.fill.to_cairo_source(cr);
    cairo_fill_preserve(cr); // Preserve this, in case we need to do the outline.

    // Check if the outline has a color. If it does, then we need to draw it.
    if (style.outline.has_opacity()) {
        style.outline.to_cairo_source(cr);
        cairo_set_line_width(cr, style.outline_width);
        cairo_stroke(cr);
    }

    cairo_restore(cr); // Restore the graphics paint settings to what we saved them to previously.
}

// Private helper function, since a lot of the markers are gonna need this.
void GraphRenderer::draw_polygon(double x, double y, double angle, uint32_t n, MarkerStyle& style){
    draw_interleaved_polygon(x, y, n, {angle}, {1}, style); // Just call to the interleaved version, so we only have one implementation.
}

void GraphRenderer::draw_marker(double x, double y, MarkerStyle& style)
{
    switch (style.type)
    {
    case '.':
    case 'O': // I don't like the way matplotlib implemented this. I will treat this as a fallthrough. Size should be tweaked by size member variable.
        draw_polygon(x, y, 0, 20, style); // Draw circle as 20 sided polygon.
        break;
    case 'V': // Fallthrough case, in case the user specified this as uppercase for some reason.
    case 'v':
        draw_polygon(x, y, 90, 3, style);
        break;
    case '3':
    case '^':
        draw_polygon(x, y, -90, 3, style);
        break;
    case '<':
        draw_polygon(x, y, 180, 3, style);
        break;
    case '>':
        draw_polygon(x, y, 0, 3, style);
        break;
    case '4':
    case 's':
    case 'S': // Fallthrough case, in case the user specified this as uppercase for some reason.
        draw_polygon(x, y, 45, 4, style);
        break;
    case '5':
    case 'p':
        draw_polygon(x, y, -18, 5, style);
        break;
    case '8':
        draw_polygon(x, y, 22.5, 8, style);
        break;
    case '*':
        draw_interleaved_polygon(x, y, 5, {-18, 18}, {1, 0.5}, style);
        break;
    case 'h':
        draw_polygon(x, y, 30, 6, style);
        break;
    case 'H':
        draw_polygon(x, y, 0, 6, style);
        break;
    case 'D':
        draw_polygon(x, y, 0, 4, style);
        break;
    case 'd':
        draw_interleaved_polygon(x, y, 2, {0, 90}, {.5, 1}, style);
        break;
    case 'P':
        draw_interleaved_polygon(x, y, 4, {-18, 18, 45}, {1, 1, .5}, style);
        break;
    case 'X':
        draw_interleaved_polygon(x, y, 4, {0, 27, 63}, {.5, 1, 1}, style);
        break;
    default:
        std::cerr << "Invalid marker type passed: '" << style.type << "'" << std::endl;
        break;
    }
}

void GraphRenderer::draw_text(double x, double y, std::string text, GraphTextStyle style) {

    cairo_save(cr); // Save the current graphics paint settings, so that we don't mess anything up that something else has going on.

    style.to_cairo_source(cr);
    
    // Get the extents of the text. 
    cairo_text_extents_t extents;
    cairo_text_extents(cr, text.c_str(), &extents);

    double full_pixel_width = extents.width + style.text_padding * 2;
    double full_pixel_height = extents.height + style.text_padding * 2;

    double sx, sy;

    if (style.absolute) {
        sx = x + plot_rect.x + plot_rect.width / 2; // Offset by plot position, and center.
        sy = y + plot_rect.y + plot_rect.height / 2; // Offset by plot position, and center.

        // Get the absolute anchor offset.
        auto [plot_offset_x, plot_offset_y] = get_offset(plot_rect.width, plot_rect.height, style.absolute_anchor);

        sx -= plot_offset_x;
        sy -= plot_offset_y;
    }
    else {
        // The coordinates are not absolute. We need to map them to the screen space.

        sx = data_to_screen_x(x);
        sy = data_to_screen_y(y);
    }

    // Get the offset from the anchors.
    auto [offset_x, offset_y] = get_offset(full_pixel_width, full_pixel_height, style.anchor);

    // Add the offset to the screen position.
    sx += offset_x;
    sy += offset_y;

    // Draw the text background, if the color has opacity.
    if (style.background_color.has_opacity()) {
        style.background_color.to_cairo_source(cr);
        cairo_rectangle(cr, sx - full_pixel_width / 2, sy - full_pixel_height / 2, full_pixel_width, full_pixel_height);
        cairo_fill(cr);
    }

    // Draw the text outline, if the color has opacity.
    if (style.outline_color.has_opacity()) {
        style.outline_color.to_cairo_source(cr);
        cairo_rectangle(cr, sx - full_pixel_width / 2, sy - full_pixel_height / 2, full_pixel_width, full_pixel_height);
        cairo_set_line_width(cr, style.outline_width);
        cairo_stroke(cr);
    }

    // Now we just have to draw the text.
    style.to_cairo_source(cr); // Reset the text style.
    cairo_move_to(cr, sx - extents.x_bearing - extents.width / 2, sy - extents.y_bearing - extents.height / 2);
    cairo_show_text(cr, text.c_str());

    cairo_restore(cr); // Restore the graphics paint settings to what we saved them to previously.
}


}