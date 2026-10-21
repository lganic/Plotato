#ifndef PLOTATO_POLAR_GRAPH_HPP
#define PLOTATO_POLAR_GRAPH_HPP

#include <cstdint>
#include <Plotato/Figure.hpp>
#include <Plotato/axis/BaseAxis.hpp>
#include <Plotato/axis/RadiusAxis.hpp>
#include <Plotato/axis/ThetaAxis.hpp>
#include <Plotato/axis/OffsetAxis.hpp>

namespace plotato {
    
class PolarGraph : public Figure{
public:
    using Figure::Figure; // Inherit constructor explicitly.

    RadiusAxis* add_radius_axis(AxisStyle style = AxisStyle());
    ThetaAxis* add_theta_axis(AxisStyle style = AxisStyle());

    void set_bounds(GraphBounds set_bounds);

private:

    bool r_axis_auto_framing = true;

    virtual double map_x(double x, double y, GraphViewport viewport);
    virtual double map_y(double x, double y, GraphViewport viewport);

    void draw_to_canvas(cairo_t *cr, uint32_t width, uint32_t height);
};

}

#endif // PLOTATO_POLAR_GRAPH_HPP
