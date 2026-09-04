#include <Plotato/util/RenderContext.hpp>
#include <Plotato/util/GraphRenderer.hpp>
#include <Plotato/util/GraphBounds.hpp>

namespace plotato {

    RenderContext::RenderContext(cairo_t* cr, GraphViewport current_viewport): cr(cr), current_viewport(current_viewport), renderer(cr, current_viewport) {}

}
