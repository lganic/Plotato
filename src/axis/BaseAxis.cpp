#include <Plotato/util/StyleStructs.hpp>
#include <Plotato/axis/BaseAxis.hpp>
#include <cmath>
#include <iostream>

namespace plotato {

int use_val_if_enabled(int val, bool enabled) {
    if (enabled) return val;

    return 0;
}

Axis::Axis() {
    side = AxisSide::BOTTOM;
    style = AxisStyle();
}

AxisPixelSize Axis::size() {

    AxisPixelSize output;

    // Just some approximations, so we can output things correctly.
    int approx_text_vertical_size = get_approx_vertical_size(style.font_size);
    int approx_text_horizontal_size = get_approx_horizontal_size(style.font_size); // Very rough approximation.

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

        break;

    case LEFT:

        output.left = 
            use_val_if_enabled(style.tick_length, style.draw_tick) +
            use_val_if_enabled(style.text_gap + approx_text_horizontal_size, style.draw_text);

        break;

    case RIGHT:

        output.right = 
            use_val_if_enabled(style.tick_length, style.draw_tick) +
            use_val_if_enabled(style.text_gap + approx_text_horizontal_size, style.draw_text);

        break;
    
    default:
        break;
    }

    return output;
}

void Axis::update_automatic_ticks(int graph_width, int graph_height) {

    float approx_content_ratio;

    int element_size;
    int content_size;

    switch (side)
    {
    case TOP:

        approx_content_ratio = 1.5;

        element_size = get_approx_horizontal_size(style.font_size);
        content_size = graph_width;

        break;
    
    case BOTTOM:

        approx_content_ratio = 1.5;

        element_size = get_approx_horizontal_size(style.font_size);
        content_size = graph_width;

        break;
    
    case LEFT:

        approx_content_ratio = 3.5;

        element_size = get_approx_vertical_size(style.font_size);
        content_size = graph_height;

        break;


    case RIGHT:

        approx_content_ratio = 3.5;

        element_size = get_approx_vertical_size(style.font_size);
        content_size = graph_height;

        break;

    default:
        break;
    }

    // The proof here is kinda simple. Assuming content size = N * element size + (N - 1) * gap size
    // The solving for N, assuming that the gap size = content ratio * element size 
    int N = std::round((content_size + approx_content_ratio * element_size) / (element_size * (approx_content_ratio + 1)));

    // Clamp N, so we always have at least 2 ticks. This keeps the arithmetic from breaking later on.
    if (N < 2) N = 2;

    style.num_ticks = -N; // Set to negative number, so we still save the number, but we don't accidentally disable the automatic ticks.
}


}