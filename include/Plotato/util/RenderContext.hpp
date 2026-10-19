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
        cairo_t* cr;
        GraphViewport current_viewport;
        GraphRenderer renderer;

        std::function<double(double, double, GraphViewport)> map_x;
        std::function<double(double, double, GraphViewport)> map_y;

        RenderContext(cairo_t* cr, GraphViewport current_viewport);
};

}

#endif // PLOTATO_RENDERCONTEXT_HPP
