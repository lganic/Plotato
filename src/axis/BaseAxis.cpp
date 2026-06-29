#include <Plotato/axis/BaseAxis.hpp>

namespace plotato {

int use_val_if_enabled(int val, bool enabled) {
    if (enabled) return val;

    return 0;
}

AxisPixelSize Axis::size() {

    AxisPixelSize output;

    // Just some approximations, so we can output things correctly.
    int approx_text_vertical_size = style.font_size;
    int approx_text_horizontal_size = approx_text_vertical_size * 4; // Very rough approximation.

    switch (side)
    {
    case TOP:
        
        output.top = 
            use_val_if_enabled(style.tick_length, style.draw_tick) + 
            use_val_if_enabled(style.text_gap + approx_text_vertical_size, style.draw_text);

        break;

    case BOTTOM:

        output.bottom = 
            use_val_if_enabled(style.tick_length, style.draw_tick) + 
            use_val_if_enabled(style.text_gap + approx_text_vertical_size, style.draw_text);

    case LEFT:

        output.left = 
            use_val_if_enabled(style.tick_length, style.draw_tick) +
            use_val_if_enabled(style.text_gap + approx_text_horizontal_size, style.draw_text);

    case RIGHT:

        output.right = 
            use_val_if_enabled(style.tick_length, style.draw_tick) +
            use_val_if_enabled(style.text_gap + approx_text_horizontal_size, style.draw_text);
    
    default:
        break;
    }

    return output;
}

}