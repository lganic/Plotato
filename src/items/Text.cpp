#include <Plotato/items/Text.hpp>
#include <Plotato/util/GraphRenderer.hpp>
#include <Plotato/util/GraphBounds.hpp>

namespace plotato {

    Text::Text(double text_x, double text_y, std::string starting_text, GraphTextStyle text_style): x(text_x), y(text_y), text(starting_text) {
        style = text_style;
    }

    GraphBounds Text::bounds() {

        GraphBounds bounds;

        if (!style.absolute) {
            bounds.adjust_with_point(x, y);
        }

        return bounds;
    }

    void Text::draw(RenderContext& ctx) {
        // Draw the text using the renderer
        ctx.renderer.draw_text(x, y, text, style);
    }
}
