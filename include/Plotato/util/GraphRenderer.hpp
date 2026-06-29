#ifndef PLOTATO_GRAPHRENDERER_HPP
#define PLOTATO_GRAPHRENDERER_HPP

#include <gtk/gtk.h>
#include <vector>
#include <string>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/StyleStructs.hpp>

namespace plotato {

struct LineStyle {
    Color color {25, 64, 230};
    double width = 2.0;
};

struct MarkerStyle {
    Color color {25, 64, 230};
    double radius = 3.0;
};

struct TextStyle {
    Color color {0, 0, 0};
    std::string font = "Sans";
    double size = 12.0;
};

class GraphRenderer {
public:
    GraphRenderer(cairo_t* cr, const GraphViewport& viewport);

    void begin_frame();
    void draw_plot_background();
    void draw_axes();
    void draw_grid();

    void draw_polyline(const std::vector<double>& x,
                       const std::vector<double>& y,
                       const LineStyle& style);

    double data_to_screen_x(double x) const;
    double data_to_screen_y(double y) const;

private:
    cairo_t* cr;

    int widget_width;
    int widget_height;

    GraphViewport viewport;

    GdkRectangle plot_rect;

    void set_color(const Color& color);
    void enable_plot_clip();
    void disable_clip();
};

}

#endif // PLOTATO_GRAPHRENDERER_HPP
