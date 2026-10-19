#include <Plotato/util/RenderContext.hpp>
#include <Plotato/util/GraphRenderer.hpp>
#include <Plotato/util/GraphBounds.hpp>

namespace plotato {

    RenderContext::RenderContext(
        cairo_t* cr,
        GraphViewport current_viewport,
        MapFunction map_x,
        MapFunction map_y
    )
        : cr(cr),
        current_viewport(current_viewport),
        renderer(cr, current_viewport),
        map_x(std::move(map_x)),
        map_y(std::move(map_y)) {}

}
