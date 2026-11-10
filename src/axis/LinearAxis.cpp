#include <Plotato/axis/LinearAxis.hpp>
#include <Plotato/util/RenderContext.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <gtk/gtk.h>
#include <cstdint>
#include <vector>
#include <iostream>

namespace plotato {

namespace detail{
    struct pg {
        double position;
        int32_t pixel_position;
    };
}

void LinearAxis::draw(RenderContext& ctx, int32_t offset_x, int32_t offset_y) {

    // First, update the number of ticks. 
    if (style.num_ticks <= 0) {
        update_automatic_ticks(ctx.current_viewport.graph_width, ctx.current_viewport.graph_height);
    }

    uint32_t num_ticks = abs(style.num_ticks);    

    // make a vector containing the pixel positions, and the graph positions.
    std::vector<detail::pg> positions(num_ticks);

    // Breaking this into two seperate loops so we aren't checking this condition inside the loop
    if (side == LEFT || side == RIGHT) {
        // Ensure first tick alignment
        positions[0] = detail::pg{ctx.current_viewport.bounds.ymin, static_cast<int32_t>(ctx.current_viewport.margin_top) + ctx.current_viewport.graph_height};

        for (uint32_t i = 1; i < num_ticks - 1; i ++) {
            double location = ctx.current_viewport.bounds.ymin + (ctx.current_viewport.bounds.ymax - ctx.current_viewport.bounds.ymin) * i / (num_ticks - 1);
            positions[i] = detail::pg{location, static_cast<int32_t>(ctx.current_viewport.map_y_to_screen(location))};
        }

        // Ensure final tick alignment.
        positions[num_ticks - 1] = detail::pg{ctx.current_viewport.bounds.ymax, static_cast<int32_t>(ctx.current_viewport.margin_top)};
    }
    else {
        // Ensure first tick alignment
        positions[0] = detail::pg{ctx.current_viewport.bounds.xmin, static_cast<int32_t>(ctx.current_viewport.margin_left)};

        for (uint32_t i = 1; i < num_ticks - 1; i ++) {
            double location = ctx.current_viewport.bounds.xmin + (ctx.current_viewport.bounds.xmax - ctx.current_viewport.bounds.xmin) * i / (num_ticks - 1);
            positions[i] = detail::pg{location, static_cast<int32_t>(ctx.current_viewport.map_x_to_screen(location))};
        }

        // Ensure final tick alignment.
        positions[num_ticks - 1] = detail::pg{ctx.current_viewport.bounds.xmax, static_cast<int32_t>(ctx.current_viewport.margin_left) + ctx.current_viewport.graph_width};
    }

    // Now we just need to plot the thing.
    // Similar older switch case logic, but only running it once to get some basic offset info, which will be re-used for each draw element.

    int32_t base_const; // Is the X coordinate of the base pos if the axis is left or right. Is the Y coordinate if the axis is top or bottom.
    int32_t tick_to_const; // Is zero if ticks are disabled. Is the X coordinate of the tick to coordinate if the axis is left or right. Is the Y coordinate of the tick to coordinate if the axis is top or bottom.
    int32_t plot_to_const; // Is zero if the plot lines are disabled. Is the X coordinate of the plot to coordinate if the axis is left or right. Is the Y coordinate of the plot to coordinate if the axis is top or bottom. (The from pos is assumed to be the base const)
    int32_t text_const; // Is zero if text is disabled. Is the X coordinate of the centered text pos if the axis is left or right. Is the Y coordinate of the centered text pos if the axis is top or bottom.
    Anchor text_anchor; // Used to properly align the text.

    switch (side)
    {
    case TOP:

        base_const = ctx.current_viewport.margin_top;
        tick_to_const = use_val_if_enabled(base_const - style.tick_length, style.draw_tick);
        plot_to_const = use_val_if_enabled(base_const + ctx.current_viewport.graph_height, style.draw_plot_lines);
        text_const = use_val_if_enabled(tick_to_const - style.text_gap, style.draw_text);
        text_anchor = Anchor::BOTTOM_CENTER;

        break;

    case BOTTOM:

        base_const = ctx.current_viewport.margin_top + ctx.current_viewport.graph_height;
        tick_to_const = use_val_if_enabled(base_const + style.tick_length, style.draw_tick);
        plot_to_const = use_val_if_enabled(ctx.current_viewport.margin_top, style.draw_plot_lines);
        text_const = use_val_if_enabled(tick_to_const + style.text_gap + get_approx_vertical_size(style.text_style.font_size), style.draw_text);
        text_anchor = Anchor::TOP_CENTER;


        break;

    case LEFT:

        base_const = ctx.current_viewport.margin_left;
        tick_to_const = use_val_if_enabled(base_const - style.tick_length, style.draw_tick);
        plot_to_const = use_val_if_enabled(base_const + ctx.current_viewport.graph_width, style.draw_plot_lines);
        text_const = use_val_if_enabled(tick_to_const - style.text_gap, style.draw_text);
        text_anchor = Anchor::MIDDLE_RIGHT;

        break;

    case RIGHT:

        base_const = ctx.current_viewport.margin_left + ctx.current_viewport.graph_width;
        tick_to_const = use_val_if_enabled(base_const + style.tick_length, style.draw_tick);
        plot_to_const = use_val_if_enabled(ctx.current_viewport.margin_left, style.draw_plot_lines);
        text_const = use_val_if_enabled(tick_to_const + style.text_gap, style.draw_text);
        text_anchor = Anchor::MIDDLE_LEFT;

        break;

    default:
        break;
    }

    if (side == LEFT || side == RIGHT) {

        // First, draw all the plot lines. We draw them first so the layering looks right.
        if (style.draw_plot_lines) {

            cairo_new_path(ctx.cr);
            style.plot_line_color.to_cairo_source(ctx.cr);

            for (auto position : positions) {
                // Use base const as X coord.
                cairo_move_to(ctx.cr, base_const, position.pixel_position);
                cairo_line_to(ctx.cr, plot_to_const, position.pixel_position);
            }

            cairo_stroke(ctx.cr);
        }

        // Draw the ticks on the axis
        if (style.draw_tick) {

            cairo_new_path(ctx.cr);
            style.tick_color.to_cairo_source(ctx.cr);

            for (auto position : positions) {
                // Use base const as from X coord
                cairo_move_to(ctx.cr, offset_x + base_const, offset_y + position.pixel_position);
                cairo_line_to(ctx.cr, offset_x + tick_to_const, offset_y + position.pixel_position);
            }

            // Now draw the base line. We can do it in the same stroke
            cairo_move_to(ctx.cr, offset_x + base_const, offset_y + static_cast<int32_t>(ctx.current_viewport.margin_top));
            cairo_line_to(ctx.cr, offset_x + base_const, offset_y + static_cast<int32_t>(ctx.current_viewport.margin_top) + static_cast<int32_t>(ctx.current_viewport.graph_height));

            cairo_stroke(ctx.cr);
        }

        // Draw the text items.
        if (style.draw_text) {

            char label[64];
            style.text_style.to_cairo_source(ctx.cr);

            for (auto position : positions) {

                // Render the string.
                snprintf(label, sizeof(label), "%.2f", position.position);

                // Get the size of the text object.
                cairo_text_extents_t extents;
                cairo_text_extents(ctx.cr, label, &extents);

                auto offset = get_offset(extents.width, extents.height, text_anchor);

                cairo_move_to(ctx.cr, offset_x + text_const + offset.first - extents.width / 2, offset_y + position.pixel_position - offset.second + extents.height / 2);
                cairo_show_text(ctx.cr, label);
            }
        }
    }
    else{
        // First, draw all the plot lines. We draw them first so the layering looks right.
        if (style.draw_plot_lines) {

            cairo_new_path(ctx.cr);
            style.plot_line_color.to_cairo_source(ctx.cr);

            for (auto position : positions) {
                // Use base const as Y coord.
                cairo_move_to(ctx.cr, position.pixel_position, base_const);
                cairo_line_to(ctx.cr, position.pixel_position, plot_to_const);
            }

            cairo_stroke(ctx.cr);
        }

        // Draw the ticks on the axis
        if (style.draw_tick) {

            cairo_new_path(ctx.cr);
            style.tick_color.to_cairo_source(ctx.cr);

            for (auto position : positions) {
                // Use base const as from Y coord
                cairo_move_to(ctx.cr, offset_x + position.pixel_position, offset_y + base_const);
                cairo_line_to(ctx.cr, offset_x + position.pixel_position, offset_y + tick_to_const);
            }

            // Now draw the base line. We can do it in the same stroke
            cairo_move_to(ctx.cr, offset_x + static_cast<int32_t>(ctx.current_viewport.margin_left), offset_y + base_const);
            cairo_line_to(ctx.cr, offset_x + static_cast<int32_t>(ctx.current_viewport.margin_left) + ctx.current_viewport.graph_width, offset_y + base_const);

            cairo_stroke(ctx.cr);
        }

        // Draw the text items.
        if (style.draw_text) {

            char label[64];
            style.text_style.to_cairo_source(ctx.cr);

            for (auto position : positions) {

                // Render the string.
                snprintf(label, sizeof(label), "%.2f", position.position);

                // Get the size of the text object.
                cairo_text_extents_t extents;
                cairo_text_extents(ctx.cr, label, &extents);

                auto offset = get_offset(extents.width, extents.height, text_anchor);

                cairo_move_to(ctx.cr, offset_x + position.pixel_position + offset.first - extents.width / 2, offset_y + text_const - offset.second + extents.height / 2);
                cairo_show_text(ctx.cr, label);
            }
        }
    }
}

}