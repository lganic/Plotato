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

        RenderContext(cairo_t* cr, GraphViewport current_viewport, GraphRenderer::MapFunction map_x, GraphRenderer::MapFunction map_y);
};

}

#endif // PLOTATO_RENDERCONTEXT_HPP
