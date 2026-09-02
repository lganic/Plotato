#ifndef PLOTATO_PLOTITEM_HPP
#define PLOTATO_PLOTITEM_HPP

#include <Plotato/util/RenderContext.hpp>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/StyleStructs.hpp>

namespace plotato {

enum class PlotType {
    NONE,
    LINEPLOT,
    SCATTERPLOT,
    TEXT,
};

class PlotItem
{
public:

    virtual PlotType type() const {
        return PlotType::NONE;
    }

    bool check_is(PlotType type);

    virtual ~PlotItem() {}

    virtual void draw(RenderContext& ctx) = 0;

    virtual GraphBounds bounds() = 0;
};

}

#endif // PLOTATO_PLOTITEM_HPP
