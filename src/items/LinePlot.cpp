#include <vector>
#include <Plotato/items/LinePlot.hpp>
#include <Plotato/util/GraphRenderer.hpp>

namespace plotato {

LinePlot::LinePlot(std::vector<double> x_data, std::vector<double> y_data): x(x_data), y(y_data){}

void LinePlot::draw(RenderContext& ctx) {

    // First, create a graph renderer, which will abstract away a lot of the drawing logic.

    GraphRenderer renderer(ctx.cr, ctx.current_viewport);

    // TODO: Don't do this. Have the user able to control this.
    LineStyle ls;

    // Pass our line data to the graph renderer. 
    renderer.draw_polyline(x, y, ls);
}

}