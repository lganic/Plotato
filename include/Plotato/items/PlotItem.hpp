#ifndef PLOTATO_PLOTITEM_HPP
#define PLOTATO_PLOTITEM_HPP

#include <Plotato/util/RenderContext.hpp>

namespace plotato {

struct Bounds {
    double min_x;
    double max_x;
    double min_y;
    double max_y;
};

class PlotItem
{
public:
    virtual ~PlotItem() {}

    virtual void draw(RenderContext& ctx);

    virtual Bounds bounds();
};

}

#endif // PLOTATO_PLOTITEM_HPP
