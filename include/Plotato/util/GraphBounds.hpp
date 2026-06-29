#ifndef PLOTATO_GRAPHBOUNDS_HPP
#define PLOTATO_GRAPHBOUNDS_HPP

namespace plotato {

// Tracks the strict bounds of the graph. To be configured by user, and passed to Graph object on init.
struct GraphBounds {
    double xmin;
    double xmax;
    double ymin;
    double ymax;
};

// Constructed by graph, and passed to graph renderers to allign everything properly, after accounting for margins.
struct GraphViewport {
    double xmin;
    double xmax;
    double ymin;
    double ymax;

    int margin_left = 60;
    int margin_right = 20;
    int margin_top = 20;
    int margin_bottom = 50;

    int graph_width;
    int graph_height;
};

}

#endif // PLOTATO_GRAPHBOUNDS_HPP