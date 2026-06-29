#include <Plotato/Graph.hpp>
#include <Plotato/items/LinePlot.hpp>
#include <Plotato/axis/LinearAxis.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <cstring>
#include <limits>
#include <cmath>
#include <algorithm>
#include "version.hpp"
#include "EmbeddedAssets.hpp"

cairo_status_t read_png_from_memory(
    void* closure,
    unsigned char* data,
    unsigned int length)
{
    MemoryPng* mem = static_cast<MemoryPng*>(closure);

    if (mem->offset + length > mem->size)
        return CAIRO_STATUS_READ_ERROR;

    std::memcpy(data, mem->data + mem->offset, length);
    mem->offset += length;

    return CAIRO_STATUS_SUCCESS;
}

namespace plotato {

// Create a new graph object when given the a drawing area, and the bounds of the graph.
Graph::Graph(GtkWidget *drawing_area, GraphStyle style)
    : area(drawing_area), style(style) // Set drawing area, and style, to skip useless default constructor.
{

    // Connect signals for events which we would need to respond to.
    g_signal_connect(area, "draw", G_CALLBACK(Graph::on_draw), this); // Is called when this is first revealed, or something else indicates that a draw is required.
    g_signal_connect(area, "size-allocate", G_CALLBACK(Graph::on_size_allocate), this); // Called when the graph element is resized.

    // Set the debug png image data from the data loaded in the object file by the linker.
    MemoryPng png {
        _binary_Icon_small_png_start,
        static_cast<std::size_t>(
            _binary_Icon_small_png_end - _binary_Icon_small_png_start
        ),
        0
    };

    // Then load the debug surface from the png data.
    debug_image = cairo_image_surface_create_from_png_stream(
        read_png_from_memory,
        &png
    );

    snprintf(version_label, sizeof(version_label), "Plotato - v%s", PLOTATO_VERSION); // Create the version string.
}

void Graph::set_bounds(GraphBounds set_bounds) {

    // First set the auto framing flags
    x_axis_auto_framing = (set_bounds.xmin == 0 && set_bounds.xmax == 0);
    y_axis_auto_framing = (set_bounds.ymin == 0 && set_bounds.ymax == 0);

    // Then lets set these bounds. If the flags above are false, then these bounds won't be touch, so the user doesn't have to call this again.
    bounds = set_bounds;
}

void Graph::draw_version_text(cairo_t* cr, int width, int height) {

    if (style.dont_draw_version_text) {
        return; // Don't proceed. Style indicates to not draw this.
    }

    // Draw the version info as a small translucent bit of text in the bottom right. (For easier debugging) TODO: Add a bit to the styling which allows you to disable this.

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 10);
    cairo_set_source_rgba(cr, 0, 0, 0, 0.5);

    cairo_text_extents_t extents;
    cairo_text_extents(cr, version_label, &extents);

    double x = width - extents.width - 4;
    double y = height - extents.height;

    cairo_set_source_rgb(cr, 0.25, 0.25, 0.25);
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, version_label);
}

// Clear the elements from the graph.
void Graph::clear() {
    std::lock_guard<std::mutex> lock(data_mutex);

    current_plot_items.clear();
}

// Queue the graph to be drawn to the surface. Use when the contents of the graph have been updated.
void Graph::draw() {

    // Queue up a draw event, which will call the on_draw method.
    gtk_widget_queue_draw(area);

}

// Plot the given data on the graph. 
void Graph::plot(const std::vector<double> &x,
                 const std::vector<double> &y)
{
    {
        std::lock_guard<std::mutex> lock(data_mutex);

        current_plot_items.emplace_back(
            std::make_unique<LinePlot>(x, y)
        );
    }
}

void Graph::add_linear_axis(AxisSide side, AxisStyle style) {

    current_axis.emplace_back(
        std::make_unique<LinearAxis>(side, style)
    );
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

    // we can calculate the centering position, since we know the plot width, and the image width is 100px
    int center_x = (width - 100) / 2;
    int center_y = (height - 100) / 2;

    // Check if the image loaded successfully
    if (cairo_surface_status(debug_image) == CAIRO_STATUS_SUCCESS) {
        // Set the image surface as the source pattern
        cairo_set_source_surface(cr, debug_image, center_x, center_y);

        // Paint the source surface onto the destination context
        cairo_paint(cr);
    } // Not really a problem if this doesn't work, since its mostly just for debugging

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

    draw_version_text(cr, width, height);
}

void Graph::draw(cairo_t *cr, int width, int height)
{
    if (current_plot_items.size() == 0) {
        // Set the placeholder image, so we know that the graph has been initialized properly, just not set to any data, or plotted. 
        draw_no_data(cr, width, height);

        return;
    }

    // Check if we need to do some auto framing.
    if (x_axis_auto_framing || y_axis_auto_framing){
        // Update the bounds first, so we know that everything is up to date before we get started on the draw process.

        double auto_min_x =  std::numeric_limits<double>::infinity();
        double auto_max_x = -std::numeric_limits<double>::infinity();

        double auto_min_y =  std::numeric_limits<double>::infinity();
        double auto_max_y = -std::numeric_limits<double>::infinity();

        for (size_t i = 0; i < current_plot_items.size(); i ++) {
            
            GraphBounds this_object_bounds = current_plot_items[i]->bounds();

            auto_min_x = std::min(auto_min_x, this_object_bounds.xmin);
            auto_max_x = std::max(auto_max_x, this_object_bounds.xmax);

            auto_min_y = std::min(auto_min_y, this_object_bounds.ymin);
            auto_max_y = std::max(auto_max_y, this_object_bounds.ymax);
        }

        // Save values to the bounds object.
        if (x_axis_auto_framing) {
            if (!std::isinf(auto_min_x)){
                bounds.xmin = auto_min_x;
            }
            if(!std::isinf(auto_max_x)) {
                bounds.xmax = auto_max_x;
            }
        }
        if (y_axis_auto_framing) {
            if (!std::isinf(auto_min_y)){
                bounds.ymin = auto_min_y;
            }
            if (!std::isinf(auto_max_y)) {
                bounds.ymax = auto_max_y;
            }
        }
        
    }

    // Set background color on the plot.
    style.background_color.to_cairo_source(cr);
    cairo_paint(cr); // Paint the background.

    // Set default margins from style.
    int left_margin = style.default_margin;
    int right_margin = style.default_margin;
    int top_margin = style.default_margin;
    int bottom_margin = style.default_margin + 10;

    // Adjust the margins based on the axis which are present
    for (size_t i = 0; i < current_axis.size(); i ++) {
        AxisPixelSize axis_size = current_axis[i]->size();

        left_margin += axis_size.left;
        right_margin += axis_size.right;
        top_margin += axis_size.top;
        bottom_margin += axis_size.bottom;
    }

    // std::cout << left_margin << "," << right_margin << "," << top_margin << "," << bottom_margin << std::endl;

    // Get some plot information based on the margin.
    int plot_x = left_margin;
    int plot_y = top_margin;
    int plot_w = width - left_margin - right_margin;
    int plot_h = height - top_margin - bottom_margin;

    if (plot_w <= 0 || plot_h <= 0)
        return; // Plot to small! Abort!

    // Plot background
    style.plot_background_color.to_cairo_source(cr);
    cairo_rectangle(cr, plot_x, plot_y, plot_w, plot_h);
    cairo_fill(cr);

    if (style.draw_border) {
        // Draw the border around the graph area. First start by setting the color.
        style.border_color.to_cairo_source(cr);
        cairo_set_line_width(cr, 1.0);
    
        cairo_move_to(cr, plot_x, plot_y); // Move to the top left of the graph area.
        cairo_line_to(cr, plot_x, plot_y + plot_h); // Line on the left hand side.
        cairo_line_to(cr, plot_x + plot_w, plot_y + plot_h); // Line on the bottom.
        cairo_line_to(cr, plot_x + plot_w, plot_y); // Line on the right hand side.
        cairo_line_to(cr, plot_x, plot_y); // Line on the top.
        cairo_stroke(cr); // Stroke the border.
    }

    // Do the actual plotting.
    // Create a new viewport / render context which we will pass to the renderers.
    GraphViewport gv;
    RenderContext rc;

    gv.xmin = bounds.xmin;
    gv.xmax = bounds.xmax;
    gv.ymin = bounds.ymin;
    gv.ymax = bounds.ymax;

    gv.graph_height = plot_h;
    gv.graph_width = plot_w;

    gv.margin_left = left_margin;
    gv.margin_right = right_margin;
    gv.margin_top = top_margin;
    gv.margin_bottom = bottom_margin;

    rc.cr = cr;
    rc.current_viewport = gv;

    std::lock_guard<std::mutex> lock(data_mutex);

    // Loop over all graph elements, and call each of their corresponding draw functions.
    for(int i = 0; i < current_plot_items.size(); i ++){
        current_plot_items[i]->draw(rc);
    }

    // Loop over all the axis, and render each to the graph.
    for(int i = 0; i < current_axis.size(); i ++){
        current_axis[i]->draw(rc);
    }

    draw_version_text(cr, width, height);
}

}