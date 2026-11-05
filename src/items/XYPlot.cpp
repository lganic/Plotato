#include <vector>
#include <algorithm>
#include <Plotato/items/XYPlot.hpp>
#include <Plotato/util/GraphRenderer.hpp>
#include <Plotato/util/GraphBounds.hpp>

namespace plotato {

XYPlot::XYPlot(std::vector<double> x_data, std::vector<double> y_data): x(x_data), y(y_data) {}
XYPlot::XYPlot(std::vector<double> y_data): y(y_data) {
    contains_x_data = false;
}

GraphBounds XYPlot::bounds(){
    auto y_result = std::minmax_element(y.begin(), y.end());

    if (contains_x_data) {
        // Find both min and max elements of the x and y array.
        auto x_result = std::minmax_element(x.begin(), x.end());

        // Return the bounds object
        return GraphBounds{*x_result.first, *x_result.second, *y_result.first, *y_result.second};
    }

    // Find calculate x min and max are just 0 and the length of the y array -1
    return GraphBounds{0, static_cast<double>(y.size()) - 1, *y_result.first, *y_result.second};
}

}