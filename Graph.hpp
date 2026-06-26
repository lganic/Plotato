#pragma once

#include <gtk/gtk.h>
#include <vector>
#include <mutex>

struct GraphBounds {
    double xmin;
    double xmax;
    double ymin;
    double ymax;
};

class Graph {
public:
    Graph(GtkWidget* drawing_area, GraphBounds bounds);

    void plot(const std::vector<double>& x,
              const std::vector<double>& y);

private:
    GtkWidget* area;
    GraphBounds bounds;

    std::vector<double> x_data;
    std::vector<double> y_data;
    std::mutex data_mutex;

    static gboolean on_draw(GtkWidget* widget, cairo_t* cr, gpointer user_data);
    static void on_size_allocate(GtkWidget* widget, GdkRectangle* allocation, gpointer user_data);

    void draw(cairo_t* cr, int width, int height);

    double map_x(double x, int plot_x, int plot_w) const;
    double map_y(double y, int plot_y, int plot_h) const;
};