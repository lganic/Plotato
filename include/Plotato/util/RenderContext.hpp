#ifndef PLOTATO_RENDERCONTEXT_HPP
#define PLOTATO_RENDERCONTEXT_HPP

#include <gtk/gtk.h>
#include <Plotato/util/GraphBounds.hpp>

namespace plotato{

struct RenderContext {
    cairo_t* cr;
    GraphViewport current_viewport;
};

}

#endif // PLOTATO_RENDERCONTEXT_HPP
