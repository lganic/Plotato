#ifndef PLOTATO_PLOTITEM_HPP
#define PLOTATO_PLOTITEM_HPP

#include <Plotato/util/RenderContext.hpp>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/StyleStructs.hpp>

namespace plotato {

class PlotItem
{
public:

    std::string name = "";

    virtual void draw_legend_element(RenderContext& rc, double x, double y, double width, double height, Color border_color, double border_width) = 0;

    virtual ~PlotItem() {}

    virtual void draw(RenderContext& ctx) = 0;

    virtual GraphBounds bounds() = 0;
};

}

#endif // PLOTATO_PLOTITEM_HPP
