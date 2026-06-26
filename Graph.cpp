#include "Graph.hpp"
#include <algorithm>
#include <cmath>

Graph::Graph(GtkWidget* drawing_area, GraphBounds initial_bounds)
    : area(drawing_area), bounds(initial_bounds)
{
    g_signal_connect(area, "draw", G_CALLBACK(Graph::on_draw), this);
    g_signal_connect(area, "size-allocate", G_CALLBACK(Graph::on_size_allocate), this);
}

void Graph::plot(const std::vector<double>& x,
                 const std::vector<double>& y)
{
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        x_data = x;
        y_data = y;
    }

    gtk_widget_queue_draw(area);
}

gboolean Graph::on_draw(GtkWidget* widget, cairo_t* cr, gpointer user_data)
{
    Graph* graph = static_cast<Graph*>(user_data);

    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);

    graph->draw(cr, allocation.width, allocation.height);
    return FALSE;
}

void Graph::on_size_allocate(GtkWidget* widget,
                             GdkRectangle* allocation,
                             gpointer user_data)
{
    Graph* graph = static_cast<Graph*>(user_data);

    // Sizing is mostly automatic because draw() receives the current size.
    // This hook exists so we can later rebuild cached surfaces, text layouts,
    // heatmap buffers, etc.
    gtk_widget_queue_draw(graph->area);
}

double Graph::map_x(double x, int plot_x, int plot_w) const
{
    double t = (x - bounds.xmin) / (bounds.xmax - bounds.xmin);
    return plot_x + t * plot_w;
}

double Graph::map_y(double y, int plot_y, int plot_h) const
{
    double t = (y - bounds.ymin) / (bounds.ymax - bounds.ymin);
    return plot_y + plot_h - t * plot_h;
}

void Graph::draw(cairo_t* cr, int width, int height)
{
    std::vector<double> x;
    std::vector<double> y;

    {
        std::lock_guard<std::mutex> lock(data_mutex);
        x = x_data;
        y = y_data;
    }

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    int left_margin = 60;
    int right_margin = 20;
    int top_margin = 20;
    int bottom_margin = 50;

    int plot_x = left_margin;
    int plot_y = top_margin;
    int plot_w = width - left_margin - right_margin;
    int plot_h = height - top_margin - bottom_margin;

    if (plot_w <= 0 || plot_h <= 0)
        return;

    // Plot background
    cairo_set_source_rgb(cr, 0.96, 0.96, 0.96);
    cairo_rectangle(cr, plot_x, plot_y, plot_w, plot_h);
    cairo_fill(cr);

    // Axes
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1.0);

    cairo_move_to(cr, plot_x, plot_y);
    cairo_line_to(cr, plot_x, plot_y + plot_h);
    cairo_line_to(cr, plot_x + plot_w, plot_y + plot_h);
    cairo_stroke(cr);

    // Simple grid + tick labels
    int ticks = 5;

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12);

    for (int i = 0; i <= ticks; i++) {
        double tx = bounds.xmin + (bounds.xmax - bounds.xmin) * i / ticks;
        double sx = map_x(tx, plot_x, plot_w);

        cairo_set_source_rgb(cr, 0.82, 0.82, 0.82);
        cairo_move_to(cr, sx, plot_y);
        cairo_line_to(cr, sx, plot_y + plot_h);
        cairo_stroke(cr);

        cairo_set_source_rgb(cr, 0, 0, 0);
        char label[64];
        snprintf(label, sizeof(label), "%.2f", tx);
        cairo_move_to(cr, sx - 12, plot_y + plot_h + 20);
        cairo_show_text(cr, label);
    }

    for (int i = 0; i <= ticks; i++) {
        double ty = bounds.ymin + (bounds.ymax - bounds.ymin) * i / ticks;
        double sy = map_y(ty, plot_y, plot_h);

        cairo_set_source_rgb(cr, 0.82, 0.82, 0.82);
        cairo_move_to(cr, plot_x, sy);
        cairo_line_to(cr, plot_x + plot_w, sy);
        cairo_stroke(cr);

        cairo_set_source_rgb(cr, 0, 0, 0);
        char label[64];
        snprintf(label, sizeof(label), "%.2f", ty);
        cairo_move_to(cr, 8, sy + 4);
        cairo_show_text(cr, label);
    }

    // Plot line
    if (x.size() < 2 || y.size() < 2)
        return;

    size_t n = std::min(x.size(), y.size());

    cairo_set_source_rgb(cr, 0.1, 0.25, 0.9);
    cairo_set_line_width(cr, 2.0);

    cairo_move_to(cr,
                  map_x(x[0], plot_x, plot_w),
                  map_y(y[0], plot_y, plot_h));

    for (size_t i = 1; i < n; i++) {
        cairo_line_to(cr,
                      map_x(x[i], plot_x, plot_w),
                      map_y(y[i], plot_y, plot_h));
    }

    cairo_stroke(cr);
}