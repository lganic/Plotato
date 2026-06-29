#ifndef PLOTATO_LINEPLOT_HPP
#define PLOTATO_LINEPLOT_HPP

#include <Plotato/items/PlotItem.hpp>
#include <vector>

namespace plotato {

class LinePlot : public PlotItem
{
    std::vector<double> x;
    std::vector<double> y;

    public:
        LinePlot(std::vector<double>, std::vector<double>);
        void draw(RenderContext& ctx);
        Bounds bounds();
};

}

#endif // PLOTATO_LINEPLOT_HPP