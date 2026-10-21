#ifndef PLOTATO_THETA_AXIS_HPP
#define PLOTATO_THETA_AXIS_HPP

#include <Plotato/axis/BaseAxis.hpp>
#include <Plotato/util/RenderContext.hpp>

namespace plotato {
    
class ThetaAxis : public Axis{
    public:
        ThetaAxis(AxisStyle style = AxisStyle())
        : Axis(AxisSide::NONE, style) {}
        void draw(RenderContext& ctx, int32_t offset_x = 0, int32_t offset_y = 0);
};

}

#endif // PLOTATO_THETA_AXIS_HPP
