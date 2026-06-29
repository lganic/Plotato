#ifndef PLOTATO_PLOTITEM_HPP
#define PLOTATO_PLOTITEM_HPP

#include <Plotato/util/RenderContext.hpp>
#include <Plotato/util/GraphBounds.hpp>

namespace plotato {

class PlotItem
{
public:
    virtual ~PlotItem() {}

    virtual void draw(RenderContext& ctx) = 0;

    virtual GraphBounds bounds() = 0;
};

}

#endif // PLOTATO_PLOTITEM_HPP
