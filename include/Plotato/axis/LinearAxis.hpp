#include <Plotato/axis/BaseAxis.hpp>
#include <Plotato/util/RenderContext.hpp>

namespace plotato {
    
class LinearAxis : public Axis{
    public:
        LinearAxis(AxisSide side, AxisStyle style = AxisStyle())
        : Axis(side, style) {}
        void draw(RenderContext& ctx, int offset_x = 0, int offset_y = 0);
};

}
