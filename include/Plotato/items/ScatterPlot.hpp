#ifndef PLOTATO_SCATTERPLOT_HPP
#define PLOTATO_SCATTERPLOT_HPP

#include <Plotato/items/PlotItem.hpp>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <vector>

namespace plotato {

class ScatterPlot : public PlotItem
{
    std::vector<double> x;
    std::vector<double> y;
    
    public:

        void draw_legend_element(RenderContext& rc, double x, double y, double width, double height, Color border_color, double border_width);

        MarkerStyle style;

        ScatterPlot(std::vector<double> x_data, std::vector<double> y_data, MarkerStyle plot_style);
        void draw(RenderContext& ctx);
        GraphBounds bounds();
};

}

#endif // PLOTATO_SCATTERPLOT_HPP