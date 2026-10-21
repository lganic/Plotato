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

    std::lock_guard<std::mutex> draw_guard(figure->draw_mutex); // Ensure that only one draw is being called at a time.

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

double Figure::map_x(double x, double y, GraphViewport viewport) {return 0;}
double Figure::map_y(double x, double y, GraphViewport viewport) {return 0;}

void Figure::draw_to_canvas(cairo_t* cr, uint32_t width, uint32_t height) {}

void Figure::draw_axis(RenderContext& rc) {

    // Draw the axis data onto some generic figure.

    std::lock_guard<std::mutex> lock(axis_mutex);

    int32_t left_axis_offset = 0;
    int32_t right_axis_offset = 0;
    int32_t top_axis_offset = 0;
    int32_t bottom_axis_offset = 0;

    // Loop over all the axis, and render each to the graph.
    // This has to use a switch case, in order to correctly stack multi axis plots.
    for(int i = 0; i < current_axis_items.size(); i ++){

        AxisPixelSize size = current_axis_items[i]->size();

        switch (current_axis_items[i]->side)
        {
        case TOP:
            current_axis_items[i] -> draw(rc, 0, -top_axis_offset);
            top_axis_offset += size.top + INTER_AXIS_PADDING;
            break;

        case BOTTOM:
            current_axis_items[i] -> draw(rc, 0, bottom_axis_offset);
            bottom_axis_offset += size.bottom + INTER_AXIS_PADDING;
            break;

        case LEFT:
            current_axis_items[i] -> draw(rc, -left_axis_offset, 0);
            left_axis_offset += size.left + INTER_AXIS_PADDING;
            break;

        case RIGHT:
            current_axis_items[i] -> draw(rc, right_axis_offset, 0);
            right_axis_offset += size.right + INTER_AXIS_PADDING;
            break;

        case NONE:
            current_axis_items[i] -> draw(rc, 0, 0);
        }
    }
}

void Figure::draw_legend(RenderContext& rc) {

    // Lets calculate the total size of the thing. 
    // To make this look good, we are going to need to know the size of all the text elements ahead of time.
    // So lets make a vector of all the text extents

    std::vector<std::unique_ptr<cairo_text_extents_t>> all_extents;

    GraphViewport viewport = rc.current_viewport;

    uint32_t plot_w = viewport.graph_width;
    uint32_t plot_h = viewport.graph_height;
    uint32_t plot_x = viewport.margin_left;
    uint32_t plot_y = viewport.margin_top;

    // Set the text style, so we have up to date font info.
    style.legend_text_style.to_cairo_source(rc.cr);

    // Do this inside of a scope, since we are accessing mutexed data.
    {
        std::lock_guard<std::mutex> plot_data_mutex(data_mutex);

        for (size_t plot_item_index = 0; plot_item_index < current_plot_items.size(); plot_item_index ++) {

            PlotItem* plot_item = current_plot_items[plot_item_index].get();

            if (plot_item->name.length() == 0) continue; // Skip plots with no name.

            // Make a new extents object.
            auto new_extents = std::make_unique<cairo_text_extents_t>();

            // Update the cairo text extents.
            cairo_text_extents(rc.cr, plot_item->name.c_str(), new_extents.get());

            // Add the text extents ptr to the array.
            all_extents.emplace_back(std::move(new_extents));
        }
    }

    // Now we can calculate the text size. 
    // I am going to use the maximum of the height, instead of each element so spacing is even.
    double max_height = 0;
    double max_width = 0;

    for (size_t extents_index = 0; extents_index < all_extents.size(); extents_index ++) {
        max_height = std::max(max_height, all_extents[extents_index]->height);
        max_width = std::max(max_width, all_extents[extents_index]->width);
    }

    // Final size calculation.
    double legend_box_width = max_width + 2 * style.legend_padding + max_height + 5;
    double legend_box_height = all_extents.size() * (max_height + style.legend_inter_object_padding) - style.legend_inter_object_padding + 2 * style.legend_padding;

    // Calculate the legend based on the anchor position.
    auto [plot_off_x, plot_off_y] = get_offset(plot_w, plot_h, style.legend_anchor);
    auto [anchor_x, anchor_y] = get_offset(legend_box_width, legend_box_height, style.legend_anchor);
    auto [padding_x, padding_y] = get_offset(2 * style.legend_offset, 2 * style.legend_offset, style.legend_anchor); // Hacky.

    // First, calculate a legend position which puts it in the exact center of the graph.
    double legend_x = plot_x + (plot_w / 2) - (legend_box_width / 2);
    double legend_y = plot_y + (plot_h / 2) - (legend_box_height / 2);

    // Then use the anchors to offset into the correct position.
    legend_x -= plot_off_x - anchor_x - padding_x;
    legend_y -= plot_off_y - anchor_y - padding_y;

    double swatch_x = legend_x + style.legend_padding;
    double text_x = swatch_x + max_height + 5;

    // Draw legend filled rectangle
    style.legend_color.to_cairo_source(rc.cr);
    cairo_rectangle(rc.cr, legend_x, legend_y, legend_box_width, legend_box_height);
    cairo_fill_preserve(rc.cr); // Fill, and keep path

    style.legend_border_color.to_cairo_source(rc.cr);
    cairo_set_line_width(rc.cr, style.legend_border_width);
    cairo_stroke(rc.cr); // Use our kept path from the rectangle to stroke

    // Now we can draw the text and the color swatches.
    // Do this inside of a scope, since we are accessing mutexed data.
    style.legend_text_style.to_cairo_source(rc.cr);
    {
        std::lock_guard<std::mutex> plot_data_mutex(data_mutex);

        for (size_t plot_item_index = 0; plot_item_index < current_plot_items.size(); plot_item_index ++) {

            PlotItem* plot_item = current_plot_items[plot_item_index].get();

            if (plot_item->name.length() == 0) continue; // Skip plots with no name.

            style.legend_text_style.to_cairo_source(rc.cr);
            cairo_move_to(rc.cr, text_x, max_height + legend_y + style.legend_offset + plot_item_index * (max_height + style.legend_inter_object_padding));
            cairo_show_text(rc.cr, plot_item->name.c_str());

            plot_item->draw_legend_element(
                rc,
                legend_x + style.legend_offset, 
                legend_y + style.legend_offset + plot_item_index * (max_height + style.legend_inter_object_padding), 
                max_height, 
                max_height,
                style.legend_border_color,
                style.legend_border_width
            );

        }
    }
}

RenderContext Figure::get_render_context(cairo_t *cr, uint32_t width, uint32_t height, bool square) {

    // Set default margins from style.
    int32_t left_margin = style.default_margin;
    int32_t right_margin = style.default_margin;
    int32_t top_margin = style.default_margin;
    int32_t bottom_margin = style.default_margin;

    if (!style.dont_draw_version_text) bottom_margin += 10;

    // Adjust the margins based on the axis which are present
    {
        std::lock_guard<std::mutex> lock(axis_mutex);

        for (size_t i = 0; i < current_axis_items.size(); i ++) {
            AxisPixelSize axis_size = current_axis_items[i]->size();

            switch (current_axis_items[i]->side)
            {
            case TOP:
                top_margin += axis_size.top;
                top_margin += INTER_AXIS_PADDING;
                break;

            case BOTTOM:
                bottom_margin += axis_size.bottom;
                bottom_margin += INTER_AXIS_PADDING;
                break;

            case LEFT:
                left_margin += axis_size.left;
                left_margin += INTER_AXIS_PADDING;
                break;
            
            case RIGHT:
                right_margin += axis_size.right;
                right_margin += INTER_AXIS_PADDING;
                break;
            
            default:
                break;
            }
        }
    }

    // Get some plot information based on the margin.
    int32_t plot_w = width - left_margin - right_margin;
    int32_t plot_h = height - top_margin - bottom_margin;

    if (square) {
        // Adjust margins to center the graph, with a square aspect ratio.
        if (plot_w < plot_h) {
            // Plot is less wide than it is tall. We should adjust the top and bottom margin.

            int32_t delta = plot_h - plot_w;

            top_margin += delta / 2;
            bottom_margin += delta / 2;
        }
        else if (plot_h < plot_w) {

            // Plot is more wide than it is tall. We should adjust the left and right margin.

            int32_t delta = plot_w - plot_h;

            left_margin += delta / 2;
            right_margin += delta / 2;
        }

        // Update the plot width and height with the new margin data. (Should be the same, or within a pixel of eachother now)
        plot_w = width - left_margin - right_margin;
        plot_h = height - top_margin - bottom_margin;
    }

    GraphViewport gv;

    gv.bounds = bounds;

    gv.graph_height = plot_h;
    gv.graph_width = plot_w;

    gv.margin_left = left_margin;
    gv.margin_right = right_margin;
    gv.margin_top = top_margin;
    gv.margin_bottom = bottom_margin;

    RenderContext rc(cr, gv, std::bind(&Figure::map_x, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), std::bind(&Figure::map_y, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    return rc;

}

void Figure::export_image(std::string filename, uint32_t width, uint32_t height) {

    std::lock_guard<std::mutex> draw_guard(draw_mutex); // Ensure that only one draw is being called at a time.

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

    cairo_t* cr = cairo_create(surface);

    // Paint white background
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    // Call the main draw command.
    draw_to_canvas(cr, width, height);

    cairo_status_t status = cairo_status(cr);
    if (status == CAIRO_STATUS_SUCCESS)
        status = cairo_surface_write_to_png(surface, filename.c_str());

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    if (status != CAIRO_STATUS_SUCCESS)
        throw std::runtime_error(cairo_status_to_string(status));
}



}