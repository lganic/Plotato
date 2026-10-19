#ifndef PLOTATO_RENDERCONTEXT_HPP
#define PLOTATO_RENDERCONTEXT_HPP

#include <gtk/gtk.h>
#include <functional>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/GraphRenderer.hpp>
#include <Plotato/util/GraphBounds.hpp>

namespace plotato{

class RenderContext {

    public:

        using MapFunction = std::function<double(double, double, GraphViewport)>;

        cairo_t* cr;
        GraphViewport current_viewport;
        GraphRenderer renderer;

        std::function<double(double, double, GraphViewport)> map_x;
        std::function<double(double, double, GraphViewport)> map_y;

        RenderContext(cairo_t* cr, GraphViewport current_viewport, MapFunction map_x, MapFunction map_y);
};

}

#endif // PLOTATO_RENDERCONTEXT_HPP
