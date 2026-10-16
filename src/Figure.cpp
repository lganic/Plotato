#include <Plotato/Figure.hpp>
#include <Plotato/items/LinePlot.hpp>
#include <Plotato/items/ScatterPlot.hpp>
#include <Plotato/axis/LinearAxis.hpp>
#include <Plotato/axis/OffsetAxis.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include "version.hpp"
#include "EmbeddedAssets.hpp"

using namespace plotato;

const uint32_t TITLE_PADDING = 10;
const uint32_t INTER_AXIS_PADDING = 5;


namespace plotato {
    
namespace detail {
    cairo_status_t read_png_from_memory(
        void* closure,
        uint8_t* data,
        uint32_t length)
    {
        MemoryPng* mem = static_cast<MemoryPng*>(closure);
    
        if (mem->offset + length > mem->size)
            return CAIRO_STATUS_READ_ERROR;
    
        std::memcpy(data, mem->data + mem->offset, length);
        mem->offset += length;
    
        return CAIRO_STATUS_SUCCESS;
    }
}

// Create a new graph object when given the a drawing area, and the bounds of the graph.
Figure::Figure(GtkWidget *drawing_area, FigureStyle style)
    : area(drawing_area), style(style) // Set drawing area, and style, to skip useless default constructor.
{

    // Connect signals for events which we would need to respond to.
    g_signal_connect(area, "draw", G_CALLBACK(Figure::on_draw), this); // Is called when this is first revealed, or something else indicates that a draw is required.
    g_signal_connect(area, "size-allocate", G_CALLBACK(Figure::on_size_allocate), this); // Called when the graph element is resized.

    // Set the debug png image data from the data loaded in the object file by the linker.
    detail::MemoryPng png {
        _binary_Icon_small_png_start,
        static_cast<std::size_t>(
            _binary_Icon_small_png_end - _binary_Icon_small_png_start
        ),
        0
    };

    // Then load the debug surface from the png data.
    debug_image = cairo_image_surface_create_from_png_stream(
        detail::read_png_from_memory,
        &png
    );

    snprintf(version_label, sizeof(version_label), "Plotato - v%s", PLOTATO_VERSION); // Create the version string.
}

void Figure::draw_version_text(cairo_t* cr, uint32_t width, uint32_t height) {

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
    double y = height - extents.height + 2;

    cairo_set_source_rgb(cr, 0.25, 0.25, 0.25);
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, version_label);
}

// Clear the elements from the graph.
void Figure::clear() {
    std::lock_guard<std::mutex> lock(data_mutex);

    current_plot_items.clear();
}

void Figure::clear_axis() {
    current_axis_items.clear();
}

void Figure::reset() {
    clear();
    clear_axis();
}

// Queue the graph to be drawn to the surface. Use when the contents of the graph have been updated.
void Figure::draw() {

    // Queue up a draw event, which will call the on_draw method.
    gtk_widget_queue_draw(area);

}

// Plot the given data on the graph. 
LinePlot* Figure::plot(const std::vector<double> &x, const std::vector<double> &y, PlotStyle style)
{
    std::lock_guard<std::mutex> lock(data_mutex);

    auto new_plot = std::make_unique<LinePlot>(x, y, style);
    LinePlot* plot = new_plot.get();

    current_plot_items.emplace_back(std::move(new_plot));

    return plot;
}

// Plot the scatter plots.
ScatterPlot* Figure::scatter(const std::vector<double> &x, const std::vector<double> &y, MarkerStyle style)
{
    std::lock_guard<std::mutex> lock(data_mutex);

    auto new_plot = std::make_unique<ScatterPlot>(x, y, style);
    ScatterPlot* plot = new_plot.get();

    current_plot_items.emplace_back(std::move(new_plot));

    return plot;
}

Title* Figure::add_axis_title(AxisSide side, std::string title, TextStyle style) {


    std::lock_guard<std::mutex> lock(axis_mutex);

    auto new_title = std::make_unique<Title>(side, title, style);
    Title* title_object = new_title.get();

    current_axis_items.emplace_back(std::move(new_title));

    return title_object;
}

Title* Figure::add_plot_title(std::string title, TextStyle style) {
    return add_axis_title(AxisSide::TOP, title, style);
}

Title* Figure::add_x_title(std::string title, TextStyle style) {
    return add_axis_title(AxisSide::BOTTOM, title, style);
}

Title* Figure::add_y_title(std::string title, TextStyle style) {
    return add_axis_title(AxisSide::LEFT, title, style);
}

Text* Figure::add_text(double x, double y, std::string text, FigureTextStyle style)
{
    std::lock_guard<std::mutex> lock(text_mutex);

    auto new_text = std::make_unique<Text>(x, y, text, style);
    Text* text_pointer = new_text.get();

    text_items.emplace_back(std::move(new_text));

    return text_pointer;
}

// Called when the figure is requested to draw.
gboolean Figure::on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    Figure *figure = static_cast<Figure *>(user_data);

    // Get the current size of the allocated area for the drawing widget.
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);

    // Pass the drawing information to the drawer.
    figure->draw_to_canvas(cr, allocation.width, allocation.height);
    return FALSE;
}

// Called when the figure is resized.
void Figure::on_size_allocate(GtkWidget *widget,
                             GdkRectangle *allocation,
                             gpointer user_data)
{
    Figure *figure = static_cast<Figure *>(user_data);

    // Sizing is mostly automatic because draw() receives the current size.
    // This hook exists so we can later rebuild cached surfaces, text layouts,
    // heatmap buffers, etc.

    // So we really just need to grab the figure element from the ref, and signal a draw event. 
    gtk_widget_queue_draw(figure->area);
}

void Figure::draw_no_data(cairo_t *cr, uint32_t width, uint32_t height) {

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
    const char* msg = "Figure Initialized - No Data";

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

double Figure::map_x(double x, uint32_t plot_x, uint32_t plot_w) {return 0;}
double Figure::map_y(double y, uint32_t plot_y, uint32_t plot_h) {return 0;}

void Figure::draw_to_canvas(cairo_t* cr, uint32_t width, uint32_t height) {}

}