#ifndef PLOTATO_BASE_AXIS_HPP
#define PLOTATO_BASE_AXIS_HPP

#include <Plotato/util/RenderContext.hpp>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <cstdint>

namespace plotato {

enum AxisSide {
    TOP, 
    RIGHT,
    BOTTOM, 
    LEFT,
    NONE
};

struct AxisPixelSize {
    uint32_t left = 0;
    uint32_t right = 0;
    uint32_t top = 0;
    uint32_t bottom = 0;
    uint32_t theta = 0;
    uint32_t r = 0;
};

struct AxisStyle {

    int32_t num_ticks = 0;

    bool draw_plot_lines = true;
    Color plot_line_color = Color(209, 209, 209);

    bool draw_tick = true;
    uint32_t tick_length = 5;
    Color tick_color = Color(0, 0, 0);
    
    bool draw_text = true;
    uint32_t text_gap = 5;
    Color text_color = Color(0, 0, 0);
    uint32_t font_size = 12;

    // TODO: Put heatmap color data here. Will be wasted space when using a non heatmap object, but will make implementation way easier.
};

struct Axis
{
    AxisStyle style;

    Axis();
    Axis(AxisSide side, AxisStyle style = AxisStyle()): side(side), style(style) {}

    virtual ~Axis() {}

    virtual void draw(RenderContext& ctx, int32_t offset_x = 0, int32_t offset_y = 0) = 0;

    virtual AxisPixelSize size();

    void update_automatic_ticks(int32_t graph_width, int32_t graph_height);

    AxisSide side;
};

}

#endif // PLOTATO_BASE_AXIS_HPP
