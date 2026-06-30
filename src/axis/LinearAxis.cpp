#include <Plotato/axis/LinearAxis.hpp>
#include <Plotato/util/RenderContext.hpp>
#include <gtk/gtk.h>
#include <iostream>

namespace plotato {

void LinearAxis::draw(RenderContext& ctx, int offset_x, int offset_y) {
    // Make a font to use for the grid axis.
    cairo_select_font_face(ctx.cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(ctx.cr, style.font_size);

    if (style.num_ticks <= 0) {
        update_automatic_ticks(ctx.current_viewport.graph_width, ctx.current_viewport.graph_height);
    }

    int num_ticks = abs(style.num_ticks);

    // Loop over each tick, and figure out what we need to do.
    
    for (int i = 0; i < num_ticks; i ++){
        // In order to make my life a little easier, I am going to have the switch case figure out where to draw, and what to draw. Then do the draw outside. 
        // Since the draw operations will be the same in all cases, just with different positions.


        // Lets first figure out where our axis is on the plot.
        double location;
        if (side == LEFT || side == RIGHT){
            location = ctx.current_viewport.ymin + (ctx.current_viewport.ymax - ctx.current_viewport.ymin) * i / (num_ticks - 1);
        }else {
            location = ctx.current_viewport.xmin + (ctx.current_viewport.xmax - ctx.current_viewport.xmin) * i / (num_ticks - 1);
        }
        
        // It would also be useful to know the text size ahead of time. So lets go ahead and do that.
        char label[64];
        snprintf(label, sizeof(label), "%.2f", location);
        
        cairo_text_extents_t extents;
        cairo_text_extents(ctx.cr, label, &extents);

        int tick_from_x, tick_from_y, tick_to_x, tick_to_y;
        int plot_from_x, plot_from_y, plot_to_x, plot_to_y;
        int text_x, text_y;
        int base_x, base_y;

        switch (side)
        {
        case TOP:

            base_x = ctx.current_viewport.map_x_to_screen(location);
            base_y = ctx.current_viewport.margin_top;

            tick_from_x = base_x;
            tick_from_y = base_y;
            tick_to_x = base_x;
            tick_to_y = base_y;
            plot_from_x = base_x;
            plot_from_y = base_y;
            plot_to_x = base_x;
            plot_to_y = base_y + ctx.current_viewport.graph_height;
            text_x = base_x;
            text_y = base_y;

            if (style.draw_tick){
                tick_to_x = base_x;
                tick_to_y = base_y - style.tick_length;
            }

            if (style.draw_text) {
                text_y = tick_to_y - style.text_gap;
                text_x = base_x - extents.width / 2;
            }
            
            break;

        case BOTTOM:
            base_x = ctx.current_viewport.map_x_to_screen(location);
            base_y = ctx.current_viewport.margin_top + ctx.current_viewport.graph_height;

            tick_from_x = base_x;
            tick_from_y = base_y;
            tick_to_x = base_x;
            tick_to_y = base_y;
            plot_from_x = base_x;
            plot_from_y = base_y;
            plot_to_x = base_x;
            plot_to_y = base_y - ctx.current_viewport.graph_height;
            text_x = base_x;
            text_y = base_y;

            if (style.draw_tick){
                tick_to_x = base_x;
                tick_to_y = base_y + style.tick_length;
            }

            if (style.draw_text) {
                text_y = tick_to_y + style.text_gap + style.font_size;
                text_x = base_x - extents.width / 2;
            }

            break;

        case LEFT:
            base_x = ctx.current_viewport.margin_left;
            base_y = ctx.current_viewport.map_y_to_screen(location);

            tick_from_x = base_x;
            tick_from_y = base_y;
            tick_to_x = base_x;
            tick_to_y = base_y;
            plot_from_x = base_x;
            plot_from_y = base_y;
            plot_to_x = base_x + ctx.current_viewport.graph_width;
            plot_to_y = base_y;
            text_x = base_x;
            text_y = base_y;

            if (style.draw_tick){
                tick_to_x = base_x - style.tick_length;
                tick_to_y = base_y;
            }

            if (style.draw_text) {
                text_x = tick_to_x - style.text_gap - extents.width;
                text_y = base_y + extents.height / 2;
            }

            break;

        case RIGHT:
            base_x = ctx.current_viewport.margin_left + ctx.current_viewport.graph_width;
            base_y = ctx.current_viewport.map_y_to_screen(location);

            tick_from_x = base_x;
            tick_from_y = base_y;
            tick_to_x = base_x;
            tick_to_y = base_y;
            plot_from_x = base_x;
            plot_from_y = base_y;
            plot_to_x = base_x - ctx.current_viewport.graph_width;
            plot_to_y = base_y;
            text_x = base_x;
            text_y = base_y;

            if (style.draw_tick){
                tick_to_x = base_x + style.tick_length;
                tick_to_y = base_y;
            }

            if (style.draw_text) {
                text_x = tick_to_x + style.text_gap;
                text_y = base_y + extents.height / 2;
            }

            break;

        default:
            break;
        }

        // Okay! Now comes the easy part if you can believe it.

        // Draw the plot line if we are flagged to do so.
        if (style.draw_plot_lines){
            style.plot_line_color.to_cairo_source(ctx.cr);

            cairo_move_to(ctx.cr, plot_from_x, plot_from_y);
            cairo_line_to(ctx.cr, plot_to_x, plot_to_y);
            cairo_stroke(ctx.cr);
        }

        // Draw the tick if we are flagged to do so. (these we actually need to apply the offset to)
        if (style.draw_tick) {
            style.tick_color.to_cairo_source(ctx.cr);

            cairo_move_to(ctx.cr, tick_from_x + offset_x, tick_from_y + offset_y);
            cairo_line_to(ctx.cr, tick_to_x + offset_x, tick_to_y + offset_y);
            cairo_stroke(ctx.cr);
        }

        // Draw the text if we are flagged to do so.
        if (style.draw_text) {
            style.text_color.to_cairo_source(ctx.cr);

            cairo_move_to(ctx.cr, text_x, text_y);
            cairo_show_text(ctx.cr, label);
        }
    }
}


}