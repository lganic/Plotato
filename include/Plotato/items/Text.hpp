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

    bool absolute = false;
    std::string text;
    Anchor anchor;

    public:

        virtual PlotType type() const {
            return PlotType::TEXT;
        }

        GraphTextStyle style;

        Text(double x, double y, std::string text, GraphTextStyle style);
        void draw(RenderContext& ctx);
        GraphBounds bounds();
};

}

#endif // PLOTATO_TEXT_HPP