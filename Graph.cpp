#include "Graph.hpp"
#include <algorithm>
#include <cmath>

// Create a new graph object when given the a drawing area, and the bounds of the graph.
Graph::Graph(GtkWidget *drawing_area, GraphBounds initial_bounds)
    : area(drawing_area), bounds(initial_bounds) // Set drawing area, and initial bounds using initializer step, to skip useless default constructor.
{

    // Connect signals for events which we would need to respond to.
    g_signal_connect(area, "draw", G_CALLBACK(Graph::on_draw), this); // Is called when this is first revealed, or something else indicates that a draw is required.
    g_signal_connect(area, "size-allocate", G_CALLBACK(Graph::on_size_allocate), this); // Called when the graph element is resized.
}

// Plot the given data on the graph. 
void Graph::plot(const std::vector<double> &x,
                 const std::vector<double> &y)
{
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        x_data = x;
        y_data = y;
    }

    // Queue up a draw event, which will call the on_draw method. TODO : Don't do this here. Subsequent calls to draw (i.e. on multiplots) will result in wasted renders.
    gtk_widget_queue_draw(area);
}

// Called when the graph is requested to draw.
gboolean Graph::on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    Graph *graph = static_cast<Graph *>(user_data);

    // Get the current size of the allocated area for the drawing widget.
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);

    // Pass the drawing information to the drawer.
    graph->draw(cr, allocation.width, allocation.height);
    return FALSE;
}

// Called when the graph is resized.
void Graph::on_size_allocate(GtkWidget *widget,
                             GdkRectangle *allocation,
                             gpointer user_data)
{
    Graph *graph = static_cast<Graph *>(user_data);

    // Sizing is mostly automatic because draw() receives the current size.
    // This hook exists so we can later rebuild cached surfaces, text layouts,
    // heatmap buffers, etc.

    // So we really just need to grab the graph element from the ref, and signal a draw event. 
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

void Graph::draw_no_data(cairo_t *cr, int width, int height) {
    // PLACEHOLDER LOGIC FOR THE PLACEHOLDER. CACHE THIS SO WE DON'T ANNIHILATE THE DISK
    cairo_surface_t *image = cairo_image_surface_create_from_png("static/Icon-small.png");

    // we can calculate the centering position, since we know the plot width, and the image width is 100px
    int center_x = (width - 100) / 2;
    int center_y = (height - 100) / 2;

    // Check if the image loaded successfully
    if (cairo_surface_status(image) == CAIRO_STATUS_SUCCESS) {
        // Set the image surface as the source pattern
        cairo_set_source_surface(cr, image, center_x, center_y);

        // Paint the source surface onto the destination context
        cairo_paint(cr);
    } // Not really a problem if this doesn't work, since its mostly just for debugging

    // Clean up the image surface memory
    cairo_surface_destroy(image);

    // Draw some text.
    const char* msg = "Graph initialized - no data";

    cairo_select_font_face(cr, "Sans",
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 16);

    cairo_text_extents_t extents;
    cairo_text_extents(cr, msg, &extents);

    double x = 50 + center_x - extents.width / 2;
    double y = 100 + center_y;

    cairo_set_source_rgb(cr, 0.25, 0.25, 0.25);
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, msg);
}

void Graph::draw(cairo_t *cr, int width, int height)
{
    std::vector<double> x;
    std::vector<double> y;

    {
        std::lock_guard<std::mutex> lock(data_mutex);
        x = x_data;
        y = y_data;
    }

    if (x.size() < 2) { // PLACEHOLDER
        // Set the placeholder image, so we know that the graph has been initialized properly, just not set to any data, or plotted. 
        draw_no_data(cr, width, height);

        return;
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

    for (int i = 0; i <= ticks; i++)
    {
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

    for (int i = 0; i <= ticks; i++)
    {
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

    for (size_t i = 1; i < n; i++)
    {
        cairo_line_to(cr,
                      map_x(x[i], plot_x, plot_w),
                      map_y(y[i], plot_y, plot_h));
    }

    cairo_stroke(cr);
}