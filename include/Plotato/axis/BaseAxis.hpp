#ifndef PLOTATO_BASEAXIS_HPP
#define PLOTATO_BASEAXIS_HPP

#include <Plotato/util/RenderContext.hpp>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/StyleStructs.hpp>

namespace plotato {

enum AxisSide {
    TOP, 
    RIGHT,
    BOTTOM, 
    LEFT
};

struct AxisPixelSize {
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;
};

struct AxisStyle {

    int num_ticks = 0;

    bool draw_plot_lines = true;
    Color plot_line_color = Color(209, 209, 209);

    bool draw_tick = true;
    int tick_length = 5;
    Color tick_color = Color(0, 0, 0);
    
    bool draw_text = true;
    int text_gap = 5;
    Color text_color = Color(0, 0, 0);
    int font_size = 12;

    // TODO: Put heatmap color data here. Will be wasted space when using a non heatmap object, but will make implementation way easier.
};

struct Axis
{
    AxisStyle style;

    Axis();
    Axis(AxisSide side, AxisStyle style = AxisStyle()): side(side), style(style) {}

    virtual ~Axis() {}

    virtual void draw(RenderContext& ctx, int offset_x = 0, int offset_y = 0) = 0;

    AxisPixelSize size();

    void update_automatic_ticks(int graph_width, int graph_height);

    AxisSide side;
};

}

#endif // PLOTATO_BASEAXIS_HPP
