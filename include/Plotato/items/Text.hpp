#ifndef PLOTATO_TEXT_HPP
#define PLOTATO_TEXT_HPP

#include <Plotato/items/PlotItem.hpp>
#include <Plotato/util/Anchor.hpp>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <string>

namespace plotato {

class Text : public PlotItem
{
    public:

        void draw_legend_element(RenderContext& rc, double x, double y, double width, double height, Color border_color, double border_width);

        std::string text;

        double x;
        double y;

        FigureTextStyle style;

        Text(double x, double y, std::string text, FigureTextStyle style);
        void draw(RenderContext& ctx);
        GraphBounds bounds();
};

}

#endif // PLOTATO_TEXT_HPP