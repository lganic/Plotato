#include <Plotato/axis/OffsetAxis.hpp>
#include <Plotato/util/RenderContext.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <gtk/gtk.h>
#include <cstdint>

namespace plotato {

void OffsetAxis::draw(RenderContext& ctx, int32_t offset_x, int32_t offset_y) {

    // First, create a new graph viewport, using the old data. 
    GraphViewport new_viewport = ctx.current_viewport;

    // Figure out which axis to offset
    if (side == TOP || side == BOTTOM) {
        new_viewport.xmin += offset;
        new_viewport.xmax += offset;
    }
    else { // Left or Right
        new_viewport.ymin += offset;
        new_viewport.ymax += offset;
    }

    // Create a new render context using the same cairo ptr, but the offset viewport.
    RenderContext new_context {
        ctx.cr,
        new_viewport
    };

    // Call the linear axis renderer, which we have inherited.
    LinearAxis::draw(new_context, offset_x, offset_y);
}

}