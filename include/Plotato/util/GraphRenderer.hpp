#ifndef PLOTATO_GRAPHRENDERER_HPP
#define PLOTATO_GRAPHRENDERER_HPP

#include <gtk/gtk.h>
#include <vector>
#include <string>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/StyleStructs.hpp>

namespace plotato {

class GraphRenderer {
public:
    GraphRenderer(cairo_t* cr, const GraphViewport& viewport);

    void begin_frame();
    void draw_plot_background();
    void draw_axes();
    void draw_grid();

    void draw_polyline(const std::vector<double>& x,
        const std::vector<double>& y,
        const PlotStyle& style);
        
        void draw_marker(double x, double y, MarkerStyle& style);
        
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
        
    void draw_interleaved_polygon(double x, double y, uint32_t n, std::vector<double> angles, std::vector<double> scales, MarkerStyle& style);
    void draw_polygon(double x, double y, double angle, uint32_t n, MarkerStyle& style);
};

}

#endif // PLOTATO_GRAPHRENDERER_HPP
