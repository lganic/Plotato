#include <Plotato/axis/Title.hpp>
#include <Plotato/util/RenderContext.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <gtk/gtk.h>
#include <cmath>
#include <cstdint>

namespace plotato {


namespace detail {

void draw_rotated_text(cairo_t *cr, std::string text, double x, double y, double angle_degrees) {

    double radians = angle_degrees * (M_PI / 180.0); // Convert degrees to radians 

    cairo_save(cr); // Save current graphics state to avoid leaking transformations
    
    cairo_translate(cr, x, y); // Move the origin directly to the target point
    
    cairo_rotate(cr, radians); // Rotate the user space around the new (0, 0) origin

    cairo_move_to(cr, 0.0, 0.0); // Position text exactly at the local origin

    cairo_show_text(cr, text.c_str()); // Draw the string

    cairo_restore(cr); // Revert translation and rotation for subsequent drawings
}

};

AxisPixelSize Title::size() {

    AxisPixelSize output;

    uint32_t approx_text_vertical_size = get_approx_vertical_size(text_style.font_size);

    switch (side)
    {
    case TOP:
        
        output.top = approx_text_vertical_size;
        break;

    case BOTTOM:

        output.bottom = approx_text_vertical_size;
        break;

    case LEFT:

        output.left = approx_text_vertical_size;
        break;

    case RIGHT:

        output.right = approx_text_vertical_size;
        break;
    
    default:
        break;
    }

    return output;
}

void Title::draw(RenderContext& ctx, int32_t offset_x, int32_t offset_y) {

    // Set the text style to the cairo source. 
    text_style.to_cairo_source(ctx.cr);

    // Figure out the extents of the new text.
    cairo_text_extents_t extents;
    cairo_text_extents(ctx.cr, title.c_str(), &extents);

    // Need to figure out how to center, and angle the text.
    uint32_t text_x;
    uint32_t text_y;
    int32_t text_angle;

    GraphViewport viewport = ctx.current_viewport;

    switch (side)
    {
    case AxisSide::TOP:
        // Top Title
        text_x = viewport.margin_left + (viewport.graph_width - extents.width) / 2 + offset_x;
        text_y = viewport.margin_top - offset_y;
        text_angle = 0;

        break;

    case AxisSide::LEFT:
        // Y Axis Title

        text_x = viewport.margin_left + offset_x;
        text_y = viewport.margin_top + (viewport.graph_height - extents.width) / 2 + offset_y;
        text_angle = -90;

        break;

    case AxisSide::RIGHT:
        // Y Axis Title, but on the right.
        
        text_x = viewport.margin_left + viewport.graph_width + offset_x;
        text_y = viewport.margin_top + (viewport.graph_height - extents.width) / 2 + offset_y;
        text_angle = 90;

        break;

    case AxisSide::BOTTOM:
        // X Axis Title

        text_x = viewport.margin_left + (viewport.graph_width - extents.width) / 2 + offset_x;
        text_y = viewport.margin_top + viewport.graph_height + offset_y;
        text_angle = 0;

        break;
    
    default:
        break;
    }

    detail::draw_rotated_text(ctx.cr, title, text_x, text_y, text_angle);
}

}