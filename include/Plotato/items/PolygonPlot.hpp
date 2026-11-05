#ifndef PLOTATO_POLYGONPLOT_HPP
#define PLOTATO_POLYGONPLOT_HPP

#include <Plotato/items/XYPlot.hpp>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <vector>

namespace plotato {

class PolygonPlot : public XYPlot
{
    public:

        void draw_legend_element(RenderContext& rc, double x, double y, double width, double height, Color border_color, double border_width);

        ShapeStyle style;

        PolygonPlot(std::vector<double> x_data, std::vector<double> y_data, ShapeStyle shape_style);
        void draw(RenderContext& ctx);
};

}

#endif // PLOTATO_POLYGONPLOT_HPP