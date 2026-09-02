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
        PlotStyle style;

        LinePlot(std::vector<double> x_data, std::vector<double> y_data, PlotStyle plot_style);
        void draw(RenderContext& ctx);
        GraphBounds bounds();
};

}

#endif // PLOTATO_LINEPLOT_HPP