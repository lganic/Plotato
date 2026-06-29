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
    int left;
    int right;
    int top;
    int bottom;
};

struct AxisStyle {

    int num_ticks = 5; // TODO: Negative will indicate automatic tick #

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

class Axis
{
public:
    AxisStyle style;

    Axis(AxisSide side, AxisStyle style = AxisStyle()): side(side), style(style) {}

    virtual ~Axis() {}

    virtual void draw(RenderContext& ctx, int offset_x = 0, int offset_y = 0) = 0;

    AxisPixelSize size();

protected:
    AxisSide side;

};

}

#endif // PLOTATO_BASEAXIS_HPP
