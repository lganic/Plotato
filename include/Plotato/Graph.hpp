#ifndef PLOTATO_GRAPH_HPP
#define PLOTATO_GRAPH_HPP

#include <cstdint>
#include <Plotato/Figure.hpp>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/axis/BaseAxis.hpp>
#include <Plotato/axis/LinearAxis.hpp>
#include <Plotato/axis/OffsetAxis.hpp>

namespace plotato {
    
class Graph : public Figure{
public:
    using Figure::Figure; // Inherit constructor explicitly.

    LinearAxis* add_linear_axis(AxisSide side, AxisStyle style = AxisStyle());
    OffsetAxis* add_offset_axis(AxisSide side, AxisStyle style = AxisStyle());
    
    void set_bounds(GraphBounds set_bounds);

private:

    bool x_axis_auto_framing = true;
    bool y_axis_auto_framing = true;

    virtual double map_x(double x, double y, GraphViewport viewport);
    virtual double map_y(double x, double y, GraphViewport viewport);

    void draw_to_canvas(cairo_t *cr, uint32_t width, uint32_t height);
};

}

#endif // PLOTATO_GRAPH_HPP
