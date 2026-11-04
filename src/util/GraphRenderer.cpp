#include <Plotato/util/GraphRenderer.hpp>
#include <Plotato/util/Anchor.hpp>
#include <cmath>
#include <utility>
#include <vector>
#include <iostream>

namespace plotato
{

GraphRenderer::GraphRenderer(cairo_t *cr, const GraphViewport &viewport, MapFunction map_x, MapFunction map_y)
    : cr(cr), viewport(viewport), map_x(std::move(map_x)), map_y(std::move(map_y)) // Default constructors for this, to optimize initialization pathways.
{
    // Assign the values of the plot rect here.
    plot_rect.x = viewport.margin_left;
    plot_rect.y = viewport.margin_top;
    plot_rect.width = viewport.graph_width;
    plot_rect.height = viewport.graph_height;
}

double GraphRenderer::data_to_screen_x(double x, double y)
{

    return map_x(x, y, viewport);

}

double GraphRenderer::data_to_screen_y(double x, double y)
{
    return map_y(x, y, viewport);
}

void GraphRenderer::set_color(const Color& color){
    cairo_set_source_rgba(cr, color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0);
}

void GraphRenderer::draw_polyline_absolute(const std::vector<double>& x, const std::vector<double>& y, const PlotStyle& style) {

    if (x.size() != y.size()) {
        std::cerr << "Could not render polyline. X and Y coordinate sizes mismatch." << std::endl;

        return;
    }

    size_t n = x.size();

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

        if (!started) {
            // Line segment not started yet. Just move to the coordinate. We will draw the line in a sec.
            cairo_move_to(cr, x[i], y[i]);
            started = true;
        } else {
            // Line segment started. So we are currently at the previous point on the line. Stroke to the next point.
            cairo_line_to(cr, x[i], y[i]);
        }
    }

    cairo_stroke(cr); // Stroke that
    cairo_restore(cr); // Restore the graphics paint settings to what we saved them to previously.
}

void GraphRenderer::draw_polyline(const std::vector<double>& x, const std::vector<double>& y, const PlotStyle& style)
{
    if (x.size() != y.size()) {
        std::cerr << "Could not render polyline. X and Y coordinate sizes mismatch." << std::endl;

        return;
    }

    size_t n = x.size();

    if (n < 2) return; // Not enough points! Abort!

    // Map the x and y data to screen coordinates.
    std::vector<double> screen_x(n);
    std::vector<double> screen_y(n);

    for (size_t i = 0; i < n; i ++) {
        screen_x[i] = data_to_screen_x(x[i], y[i]);
        screen_y[i] = data_to_screen_y(x[i], y[i]);
    }

    // Pass to absolute screen coordinates.
    draw_polyline_absolute(screen_x, screen_y, style);
}

void GraphRenderer::draw_polygon_absolute(const std::vector<double>& x, const std::vector<double>& y, ShapeStyle& style) {

    cairo_save(cr); // Save the current graphics paint settings, so that we don't mess anything up that something else has going on.

    if (x.size() != y.size()) {
        std::cerr << "Could not render polygon. X and Y coordinate sizes mismatch." << std::endl;

        return;
    }

    size_t n = x.size();

    if (n < 3) return; // Not enough points! Abort!

    bool started = false;

    // First make the path.
    for (size_t i = 0; i < n; i ++) {

        if (!started) {
            // Loop not started yet. Just move to the coordinate.
            cairo_move_to(cr, x[i], y[i]);
        }
        else {
            // Loop started. Just keep going.
            cairo_line_to(cr, x[i], y[i]);
        }

        started = true;
    }

    // Path complete. Close it.
    cairo_close_path(cr);

    // Now lets stroke it.
    if (style.fill.has_opacity()) {
        // Fill the background.
        style.fill.to_cairo_source(cr);
        cairo_fill_preserve(cr);
    }

    // Check if we should do a background as well.
    if (style.outline.has_opacity()) {
        // Stroke the outline.
        style.outline.to_cairo_source(cr);
        cairo_stroke_preserve(cr);
    }

    cairo_new_path(cr); // Clear out the preserved path. Since we use preserve twice to cover all bases.

    cairo_restore(cr); // Restore the graphics paint settings to what we saved them to previously.
}

void GraphRenderer::draw_polygon(const std::vector<double>& x, const std::vector<double>& y, ShapeStyle& style) {
    if (x.size() != y.size()) {
        std::cerr << "Could not render polygon. X and Y coordinate sizes mismatch." << std::endl;

        return;
    }

    size_t n = x.size();

    if (n < 3) return; // Not enough points! Abort!

    // Map the x and y data to screen coordinates.
    std::vector<double> screen_x(n);
    std::vector<double> screen_y(n);

    for (size_t i = 0; i < n; i ++) {
        screen_x[i] = data_to_screen_x(x[i], y[i]);
        screen_y[i] = data_to_screen_y(x[i], y[i]);
    }

    // Pass to absolute screen coordinates.
    draw_polygon_absolute(screen_x, screen_y, style);
}

std::pair<std::vector<double>, std::vector<double>> interleave_polygons(double poly_x, double poly_y, double angle, double radius, uint32_t n, std::vector<double> angles_degrees, std::vector<double> scales) {
    if (angles_degrees.size() != scales.size()) {
        std::cerr << "Could not calculate interleaved polygon. angle degrees and scales mismatch." << std::endl;

        return std::make_pair(std::vector<double>(), std::vector<double>());
    }

    if (n * angles_degrees.size() < 3) return std::make_pair(std::vector<double>(), std::vector<double>()); // Shouldn't do anything.

    double angle_offset_radians = angle * M_PI / 180.0;

    std::vector<double> angles_radians(angles_degrees.size());
    std::vector<double> scaled_sizes(scales.size());

    std::vector<double> out_x(n * angles_degrees.size());
    std::vector<double> out_y(n * angles_degrees.size());
    
    // First, cast all the angles to radians, and scale the radius to the scale given in the scale vector
    for(size_t i = 0; i < angles_degrees.size(); i ++) {
        angles_radians[i] = angles_degrees[i] * M_PI / 180.0;
        scaled_sizes[i] = scales[i] * radius;
    }

    uint32_t out_index = 0;

    for(uint32_t i_vertex = 0; i_vertex < n; i_vertex++)
    {
        double angle_offset = angle_offset_radians + static_cast<double>(i_vertex) * 2.0 * M_PI / static_cast<double>(n);

        for (size_t polygon_index = 0; polygon_index < scaled_sizes.size(); polygon_index ++){
            out_x[out_index] = poly_x + scaled_sizes[polygon_index] * std::cos(angles_radians[polygon_index] + angle_offset);
            out_y[out_index] = poly_y + scaled_sizes[polygon_index] * std::sin(angles_radians[polygon_index] + angle_offset);

            out_index ++;            
        }
    }

    return std::make_pair(out_x, out_y);
}

void GraphRenderer::draw_regular_polygons_interleaved_absolute(double poly_x, double poly_y, double angle, double radius, uint32_t n, std::vector<double> angles_degrees, std::vector<double> scales, ShapeStyle& style) {
    std::pair<std::vector<double>, std::vector<double>> interleaved = interleave_polygons(poly_x, poly_y, angle, radius, n, angles_degrees, scales);

    draw_polygon_absolute(interleaved.first, interleaved.second, style);
}

void GraphRenderer::draw_regular_polygons_interleaved(double poly_x, double poly_y, double angle, double radius, uint32_t n, std::vector<double> angles_degrees, std::vector<double> scales, ShapeStyle& style) {
    std::pair<std::vector<double>, std::vector<double>> interleaved = interleave_polygons(poly_x, poly_y, angle, radius, n, angles_degrees, scales);

    draw_polygon(interleaved.first, interleaved.second, style);
}

// Private helper function, since a lot of the markers are gonna need this.
void GraphRenderer::draw_regular_polygon_absolute(double x, double y, double angle, double radius, uint32_t n, ShapeStyle& style){
    draw_regular_polygons_interleaved_absolute(x, y, angle, radius, n, {0}, {1}, style); // Just call to the interleaved version, so we only have one implementation.
}

void GraphRenderer::draw_regular_polygon(double x, double y, double angle, double radius, uint32_t n, ShapeStyle& style) {
    draw_regular_polygons_interleaved(x, y, angle, radius, n, {0}, {1}, style); // Just call to the interleaved version, so we only have one implementation.
}

// Draw a circle at exact pixel coordinates, with exact pixel sizes.
void GraphRenderer::draw_circle_absolute(double absolute_x, double absolute_y, double pixel_radius, ShapeStyle& style, double margin) {

    // First, calculate how many segments we need to render this. 
    // We do this based on an approximate solution to this equation:

    // r*(1-sqrt(2pi/(N * sin(360 / N)))) < M

    // Where M is the maximum amount of pixel margin that the polygon is allowed to be off at the vertexes.
    // To derive that, just solve first for the radius of a polygon where the area matches the circle equivalent
    // then re-arrange to find rp-rc < M where rp is radius of polygon, and rc is radius of circle.

    // You will see my rp formula in a sec.

    // Approximate solution to N is this:
    uint32_t num_faces = std::round(2 * M_PI / std::sqrt(6 * (1 - std::pow(1.0 + margin / pixel_radius, -2))));

    std::cout << num_faces << std::endl;

    // Then we can find the radius of the polygon like so:
    double radius_polygon = pixel_radius * std::sqrt(2 * M_PI / (num_faces * std::sin(2 * M_PI / num_faces)));

    draw_regular_polygon_absolute(absolute_x, absolute_y, 0, radius_polygon, num_faces, style);
}

void GraphRenderer::draw_circle(double x, double y, double radius, ShapeStyle& style, double margin) {

    // Lets do the same circle calculation as before to figure out the number of faces. 
    // We will just do this once per axis, then figure out which axis is best.

    double delta_x_pixels = std::abs(data_to_screen_x(x + radius, y + radius) - data_to_screen_x(x, y)); // Offset x and y by radius, and see how far offset in x direction.
    double delta_y_pixels = std::abs(data_to_screen_y(x + radius, y + radius) - data_to_screen_y(x, y)); // Offset x and y by radius, and see how far offset in y direction.

    delta_x_pixels *= 0.707106; // Get side length from hypotenuse length
    delta_y_pixels *= 0.707106; // Get side length from hypotenuse length

    uint32_t num_faces_x = std::round(2 * M_PI / std::sqrt(6 * (1 - std::pow(1.0 + margin / delta_x_pixels, -2))));
    uint32_t num_faces_y = std::round(2 * M_PI / std::sqrt(6 * (1 - std::pow(1.0 + margin / delta_y_pixels, -2))));

    uint32_t num_faces = std::max(num_faces_x, num_faces_y);

    double radius_polygon = radius * std::sqrt(2 * M_PI / (num_faces * std::sin(2 * M_PI / num_faces)));

    draw_regular_polygon(x, y, 0, radius_polygon, num_faces, style);
}

void GraphRenderer::draw_marker_absolute(double absolute_x, double absolute_y, MarkerStyle& style)
{
    switch (style.type)
    {
    case '.':
    case 'O': // I don't like the way matplotlib implemented this. I will treat this as a fallthrough. Size should be tweaked by size member variable.
        draw_circle_absolute(absolute_x, absolute_y, style.size, style);
        break;
    case 'V': // Fallthrough case, in case the user specified this as uppercase for some reason.
    case 'v':
        draw_regular_polygon_absolute(absolute_x, absolute_y, 90, style.size, 3, style);
        break;
    case '3':
    case '^':
        draw_regular_polygon_absolute(absolute_x, absolute_y, -90, style.size, 3, style);
        break;
    case '<':
        draw_regular_polygon_absolute(absolute_x, absolute_y, 180, style.size, 3, style);
        break;
    case '>':
        draw_regular_polygon_absolute(absolute_x, absolute_y, 0, style.size, 3, style);
        break;
    case '4':
    case 's':
    case 'S': // Fallthrough case, in case the user specified this as uppercase for some reason.
        draw_regular_polygon_absolute(absolute_x, absolute_y, 45, style.size, 4, style);
        break;
    case '5':
    case 'p':
        draw_regular_polygon_absolute(absolute_x, absolute_y, -18, style.size, 5, style);
        break;
    case '8':
        draw_regular_polygon_absolute(absolute_x, absolute_y, 22.5, style.size, 8, style);
        break;
    case '*':
        draw_regular_polygons_interleaved_absolute(absolute_x, absolute_y, 0, style.size, 5, {-18, 18}, {1, 0.5}, style);
        break;
    case 'h':
        draw_regular_polygon_absolute(absolute_x, absolute_y, 30, style.size, 6, style);
        break;
    case 'H':
        draw_regular_polygon_absolute(absolute_x, absolute_y, 0, style.size, 6, style);
        break;
    case 'D':
        draw_regular_polygon_absolute(absolute_x, absolute_y, 0, style.size, 4, style);
        break;
    case 'd':
        draw_regular_polygons_interleaved_absolute(absolute_x, absolute_y, 0, style.size, 2, {0, 90}, {.5, 1}, style);
        break;
    case '+':
    case 'P':
        draw_regular_polygons_interleaved_absolute(absolute_x, absolute_y, 0, style.size, 4, {-18, 18, 45}, {1, 1, .5}, style);
        break;
    case 'X':
        draw_regular_polygons_interleaved_absolute(absolute_x, absolute_y, 0, style.size, 4, {0, 27, 63}, {.5, 1, 1}, style);
        break;
    default:
        std::cerr << "Invalid marker type passed: '" << style.type << "'" << std::endl;
        break;
    }
}

void GraphRenderer::draw_marker(double x, double y, MarkerStyle& style)
{

    double mapped_x = data_to_screen_x(x, y);
    double mapped_y = data_to_screen_y(x, y);

    draw_marker_absolute(mapped_x, mapped_y, style);

}

void GraphRenderer::draw_text(double x, double y, std::string text, FigureTextStyle style) {

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

        sx = data_to_screen_x(x, y);
        sy = data_to_screen_y(x, y);
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