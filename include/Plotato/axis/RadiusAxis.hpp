#ifndef PLOTATO_RADIUS_AXIS_HPP
#define PLOTATO_RADIUS_AXIS_HPP

#include <Plotato/axis/BaseAxis.hpp>
#include <Plotato/util/RenderContext.hpp>

namespace plotato {
    
class RadiusAxis : public Axis{
    public:
        RadiusAxis(AxisStyle style = AxisStyle())
        : Axis(AxisSide::NONE, style) {}
        void draw(RenderContext& ctx, int32_t offset_x = 0, int32_t offset_y = 0);
};

}

#endif // PLOTATO_RADIUS_AXIS_HPP
