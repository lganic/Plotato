#ifndef PLOTATO_GRAPH_HPP
#define PLOTATO_GRAPH_HPP

#include <gtk/gtk.h>
#include <vector>
#include <mutex>
#include <memory>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/RenderContext.hpp>
#include <Plotato/items/PlotItem.hpp>

// This is used to store debug image data.
struct MemoryPng
{
    const unsigned char* data;
    std::size_t size;
    std::size_t offset;
};

namespace plotato {

class Graph {
public:
    Graph(GtkWidget* drawing_area, GraphBounds bounds);

    void clear();
    void draw();

    void plot(const std::vector<double>& x,
              const std::vector<double>& y);

private:

    cairo_surface_t* debug_image = nullptr;
    char version_label[64];

    GtkWidget* area;
    GraphBounds bounds;

    std::vector<std::unique_ptr<PlotItem>> current_plot_items;

    std::mutex data_mutex;

    static gboolean on_draw(GtkWidget* widget, cairo_t* cr, gpointer user_data);
    static void on_size_allocate(GtkWidget* widget, GdkRectangle* allocation, gpointer user_data);
    
    void draw_no_data(cairo_t* cr, int width, int height);
    void draw(cairo_t* cr, int width, int height);

    double map_x(double x, int plot_x, int plot_w) const;
    double map_y(double y, int plot_y, int plot_h) const;

    void draw_version_text(cairo_t* cr, int width, int height);
};

}

#endif // PLOTATO_GRAPH_HPP
