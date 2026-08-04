#ifndef PLOTATO_LINEARAXIS_HPP
#define PLOTATO_LINEARAXIS_HPP

#include <Plotato/axis/BaseAxis.hpp>
#include <Plotato/util/RenderContext.hpp>

namespace plotato {
    
class LinearAxis : public Axis{
    public:
        LinearAxis(AxisSide side, AxisStyle style = AxisStyle())
        : Axis(side, style) {}
        void draw(RenderContext& ctx, int32_t offset_x = 0, int32_t offset_y = 0);
};

}

#endif // PLOTATO_LINEARAXIS_HPP
