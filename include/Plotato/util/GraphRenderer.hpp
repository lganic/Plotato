#ifndef PLOTATO_GRAPHRENDERER_HPP
#define PLOTATO_GRAPHRENDERER_HPP

#include <gtk/gtk.h>
#include <vector>
#include <functional>
#include <string>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/StyleStructs.hpp>

namespace plotato {

class GraphRenderer {
public:

    using MapFunction = std::function<double(double, double, GraphViewport)>;

    GraphRenderer(cairo_t* cr, const GraphViewport& viewport, MapFunction map_x, MapFunction map_y);

    void begin_frame();
    void draw_plot_background();
    void draw_axes();
    void draw_grid();

    void draw_polyline_absolute(const std::vector<double>& x, const std::vector<double>& y, const PlotStyle& style);
    void draw_polyline(const std::vector<double>& x, const std::vector<double>& y, const PlotStyle& style);
    void draw_polygon_absolute(const std::vector<double>& x, const std::vector<double>& y, ShapeStyle& style);
    void draw_polygon(const std::vector<double>& x, const std::vector<double>& y, ShapeStyle& style);
    void draw_circle_absolute(double absolute_x, double absolute_y, double pixel_radius, ShapeStyle& style, double margin = 0.1);
    void draw_circle(double x, double y, double radius, ShapeStyle& style, double margin = 0.1);
    
    void draw_marker_absolute(double absolute_x, double absolute_y, MarkerStyle& style);
    void draw_marker(double x, double y, MarkerStyle& style);

    void draw_text(double x, double y, std::string text, FigureTextStyle style);
    
    double data_to_screen_x(double x, double y);
    double data_to_screen_y(double x, double y);
        
private:
    cairo_t* cr;
    
    GraphViewport viewport;
    
    GdkRectangle plot_rect;

    MapFunction map_x;
    MapFunction map_y;
    
    std::pair<std::vector<double>, std::vector<double>> map_coordinates(std::vector<double> x, std::vector<double> y);

    void set_color(const Color& color);
    void enable_plot_clip();
    void disable_clip();
        
    void draw_regular_polygons_interleaved_absolute(double absolute_x, double absolute_y, double angle, double radius, uint32_t n, std::vector<double> angles, std::vector<double> scales, ShapeStyle& style);
    void draw_regular_polygon_absolute(double absolute_x, double absolute_y, double angle, double radius, uint32_t n, ShapeStyle& style);

    void draw_regular_polygons_interleaved(double x, double y, double angle, double radius, uint32_t n, std::vector<double> angles, std::vector<double> scales, ShapeStyle& style);
    void draw_regular_polygon(double x, double y, double angle, double radius, uint32_t n, ShapeStyle& style);
};

}

#endif // PLOTATO_GRAPHRENDERER_HPP
