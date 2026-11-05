#ifndef PLOTATO_CIRCLEPLOT_HPP
#define PLOTATO_CIRCLEPLOT_HPP

#include <Plotato/items/PlotItem.hpp>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <vector>

namespace plotato {

class CirclePlot : public PlotItem
{

    double x;
    double y;
    double radius;

    
    public:
        double margin = 0.1;

        void draw_legend_element(RenderContext& rc, double x, double y, double width, double height, Color border_color, double border_width);

        ShapeStyle style;

        CirclePlot(double x, double y, double radius, ShapeStyle shape_style);
        void draw(RenderContext& ctx);
        GraphBounds bounds();
};

}

#endif // PLOTATO_CIRCLEPLOT_HPP