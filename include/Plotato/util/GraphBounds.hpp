#ifndef PLOTATO_GRAPHBOUNDS_HPP
#define PLOTATO_GRAPHBOUNDS_HPP

#include <limits>

namespace plotato {

// Tracks the strict bounds of the graph. To be configured by user, and passed to Graph object on init.
struct GraphBounds {
    double xmin =  std::numeric_limits<double>::infinity();
    double xmax = -std::numeric_limits<double>::infinity();
    double ymin =  std::numeric_limits<double>::infinity();
    double ymax = -std::numeric_limits<double>::infinity();

    void adjust_with_bounds(GraphBounds bounds);
    void adjust_with_point(double x, double y);
    bool overlap(GraphBounds target);
};

// Constructed by graph, and passed to graph renderers to allign everything properly, after accounting for margins.
struct GraphViewport {
    GraphBounds bounds;

    uint32_t margin_left = 10;
    uint32_t margin_right = 10;
    uint32_t margin_top = 10;
    uint32_t margin_bottom = 10;

    int32_t graph_width;
    int32_t graph_height;

    uint32_t map_x_to_screen(double x);
    uint32_t map_y_to_screen(double y);
};

}

#endif // PLOTATO_GRAPHBOUNDS_HPP