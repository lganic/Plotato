#ifndef PLOTATO_SCATTERPLOT_HPP
#define PLOTATO_SCATTERPLOT_HPP

#include <Plotato/items/XYPlot.hpp>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <vector>

namespace plotato {

class ScatterPlot : public XYPlot
{
    public:

        void draw_legend_element(RenderContext& rc, double x, double y, double width, double height, Color border_color, double border_width);

        MarkerStyle style;

        ScatterPlot() = default;
        ScatterPlot(std::vector<double> x_data, std::vector<double> y_data, MarkerStyle plot_style);
        ScatterPlot(std::vector<double> y_data, MarkerStyle plot_style);
        void draw(RenderContext& ctx);
};

}

#endif // PLOTATO_SCATTERPLOT_HPP