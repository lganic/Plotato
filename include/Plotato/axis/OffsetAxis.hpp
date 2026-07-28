#ifndef PLOTATO_OFFSETAXIS_HPP
#define PLOTATO_OFFSETAXIS_HPP

#include <Plotato/axis/LinearAxis.hpp>
#include <Plotato/util/RenderContext.hpp>

namespace plotato {
    
class OffsetAxis : public LinearAxis{
    public:

        double offset = 0;

        OffsetAxis(AxisSide side, AxisStyle style = AxisStyle())
        : LinearAxis(side, style) {}
        void draw(RenderContext& ctx, int32_t offset_x = 0, int32_t offset_y = 0);
};

}

#endif // PLOTATO_OFFSETAXIS_HPP
