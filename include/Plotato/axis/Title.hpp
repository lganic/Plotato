#ifndef PLOTATO_TITLE_HPP
#define PLOTATO_TITLE_HPP

#include <Plotato/axis/BaseAxis.hpp>
#include <Plotato/util/RenderContext.hpp>
#include <string>

namespace plotato {
    
class Title : public Axis{
    public:

        std::string title;
        TextStyle text_style;
    
        Title(AxisSide side, std::string title, TextStyle text_style = TextStyle()):
        Axis(side), title(title), text_style(text_style) {}

        void draw(RenderContext& ctx, int32_t offset_x = 0, int32_t offset_y = 0);
        AxisPixelSize size();
};

}

#endif // PLOTATO_TITLE_HPP
