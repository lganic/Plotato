#ifndef PLOTATO_LINEPLOT_HPP
#define PLOTATO_LINEPLOT_HPP

#include <Plotato/items/PlotItem.hpp>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <vector>

namespace plotato {

class LinePlot : public PlotItem
{
    std::vector<double> x;
    std::vector<double> y;
    
    public:

        bool marker_active = false;

        void draw_legend_element(RenderContext& rc, double x, double y, double width, double height, Color border_color, double border_width);

        PlotStyle style;

        LinePlot(std::vector<double> x_data, std::vector<double> y_data, PlotStyle plot_style);
        void draw(RenderContext& ctx);
        GraphBounds bounds();

        void marker(MarkerStyle style = MarkerStyle());
};

}

#endif // PLOTATO_LINEPLOT_HPP