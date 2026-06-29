#pragma once

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
};

}