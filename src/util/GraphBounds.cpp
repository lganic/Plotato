#include <Plotato/util/GraphBounds.hpp>

int map_single_axis(double val, double min_val, double max_val, int size, int offset){

    double t = (val - min_val) / (max_val - min_val);

    return size * t + offset;
}

namespace plotato {

int GraphViewport::map_x_to_screen(double x) {
    return map_single_axis(x, xmin, xmax, graph_width, margin_left);
}

int GraphViewport::map_y_to_screen(double y) {
    return map_single_axis(y, ymin, ymax, graph_height, margin_top);
}

}