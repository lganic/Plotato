#pragma once

#include <gtk/gtk.h>
#include <Plotato/util/GraphBounds.hpp>

namespace plotato{

struct RenderContext {
    cairo_t* cr;
    GraphViewport current_viewport;
};

}
