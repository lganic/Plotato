#ifndef PLOTATO_GRAPH_HPP
#define PLOTATO_GRAPH_HPP

#include <gtk/gtk.h>
#include <vector>
#include <mutex>
#include <memory>
#include <string>
#include <cstdint>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/RenderContext.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <Plotato/items/PlotItem.hpp>
#include <Plotato/axis/BaseAxis.hpp>
#include <Plotato/axis/LinearAxis.hpp>

namespace plotato {
    
namespace detail {
    // This is used to store debug image data.
    struct MemoryPng
    {
        const unsigned char* data;
        std::size_t size;
        std::size_t offset;
    };
}

struct GraphTitle {
    bool exists = false;
    std::string title;
    TextStyle style;
};

struct GraphStyle {
    Color background_color = Color(255, 255, 255);

    Color plot_background_color = Color(245, 245, 245);

    bool draw_border = true;
    Color border_color = Color(0, 0, 0);

    uint32_t default_margin = 10;

    bool dont_draw_version_text = false;
};


class Graph {
public:
    Graph(GtkWidget* drawing_area, GraphStyle style = GraphStyle());

    void clear();
    void reset();
    void draw();

    LinearAxis* add_linear_axis(AxisSide side, AxisStyle style = AxisStyle());
    
    void clear_axis();

    void plot(const std::vector<double>& x,
              const std::vector<double>& y);

    void set_bounds(GraphBounds set_bounds);

    GraphTitle& add_plot_title(std::string title, TextStyle style = TextStyle());
    GraphTitle& add_x_title(std::string title, TextStyle style = TextStyle());
    GraphTitle& add_y_title(std::string title, TextStyle style = TextStyle());

    GraphStyle style;

    GraphTitle plot_title;
    GraphTitle y_title;
    GraphTitle x_title;

private:

    cairo_surface_t* debug_image = nullptr;
    char version_label[64];

    GtkWidget* area;
    GraphBounds bounds; // This is the bounds object which will be used / updated at runtime. The manual bounds below, and the framing flags control this value.

    bool x_axis_auto_framing = true;
    bool y_axis_auto_framing = true;

    std::vector<std::unique_ptr<PlotItem>> current_plot_items;
    std::vector<std::unique_ptr<Axis>> current_axis;

    std::mutex data_mutex;
    std::mutex axis_mutex;

    static gboolean on_draw(GtkWidget* widget, cairo_t* cr, gpointer user_data);
    static void on_size_allocate(GtkWidget* widget, GdkRectangle* allocation, gpointer user_data);
    
    void draw_no_data(cairo_t* cr, uint32_t width, uint32_t height);
    void draw(cairo_t* cr, uint32_t width, uint32_t height);

    double map_x(double x, uint32_t plot_x, uint32_t plot_w) const;
    double map_y(double y, uint32_t plot_y, uint32_t plot_h) const;

    void draw_version_text(cairo_t* cr, uint32_t width, uint32_t height);
};

}

#endif // PLOTATO_GRAPH_HPP
