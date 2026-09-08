#include <Plotato/util/GraphBounds.hpp>
#include <algorithm>

namespace plotato {

namespace detail {

    int map_single_axis(double val, double min_val, double max_val, int size, int offset){

    double t = (val - min_val) / (max_val - min_val);

    return size * t + offset;
}

}

void GraphBounds::adjust_with_bounds(GraphBounds bounds) {

    xmin = std::min(xmin, bounds.xmin);
    xmax = std::max(xmax, bounds.xmax);

    ymin = std::min(ymin, bounds.ymin);
    ymax = std::max(ymax, bounds.ymax);

}

void GraphBounds::adjust_with_point(double x, double y) {

    xmin = std::min(xmin, x);
    xmax = std::max(xmax, x);

    ymin = std::min(ymin, y);
    ymax = std::max(ymax, y);
    
}

int GraphViewport::map_x_to_screen(double x) {
    return detail::map_single_axis(x, bounds.xmin, bounds.xmax, graph_width, margin_left);
}

int GraphViewport::map_y_to_screen(double y) {
    return detail::map_single_axis(y, bounds.ymax, bounds.ymin, graph_height, margin_top); // ymin and ymax are flipped here. Its a janky solution to correctly flip the vertical axis.
}

}