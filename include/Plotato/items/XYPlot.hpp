#ifndef PLOTATO_XYPLOT_HPP
#define PLOTATO_XYPLOT_HPP

#include <Plotato/items/PlotItem.hpp>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <vector>

namespace plotato {

class XYPlot : public PlotItem
{
    protected:

        std::vector<double> x;
        std::vector<double> y;

        bool contains_x_data = true;
    
    public:
    
        XYPlot() = default;
        XYPlot(std::vector<double> x_data, std::vector<double> y_data);
        XYPlot(std::vector<double> y_data);
        
        GraphBounds bounds(); // This should not be overridden by the child. We can handle it.
        
        virtual void draw(RenderContext& ctx) = 0;
        virtual void draw_legend_element(RenderContext& rc, double x, double y, double width, double height, Color border_color, double border_width) = 0;
};

}

#endif // PLOTATO_XYPLOT_HPP