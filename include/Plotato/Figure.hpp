#ifndef PLOTATO_FIGURE_HPP
#define PLOTATO_FIGURE_HPP

#include <gtk/gtk.h>
#include <vector>
#include <mutex>
#include <memory>
#include <string>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <Plotato/util/GraphBounds.hpp>
#include <Plotato/util/RenderContext.hpp>
#include <Plotato/util/StyleStructs.hpp>
#include <Plotato/items/PlotItem.hpp>
#include <Plotato/items/LinePlot.hpp>
#include <Plotato/items/ScatterPlot.hpp>
#include <Plotato/items/PolygonPlot.hpp>
#include <Plotato/items/CirclePlot.hpp>
#include <Plotato/items/Text.hpp>
#include <Plotato/axis/BaseAxis.hpp>
#include <Plotato/axis/LinearAxis.hpp>
#include <Plotato/axis/OffsetAxis.hpp>
#include <Plotato/axis/Title.hpp>

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

struct MarginInformation {
    uint16_t left_margin;
    uint16_t right_margin;
    uint16_t top_margin;
    uint16_t bottom_margin;
};

struct FigureStyle {
    Color background_color = Color(255, 255, 255);

    Color plot_background_color = Color(245, 245, 245);

    bool draw_border = true;
    Color border_color = Color(0, 0, 0);

    uint32_t default_margin = 10;

    bool draw_legend = false;
    TextStyle legend_text_style = TextStyle(14);
    uint16_t legend_padding = 4;
    uint16_t legend_inter_object_padding = 4; // TODO : Rename this. Don't like this name.
    int16_t legend_offset_left = 0; // Offset the legend left and right.
    int16_t legend_offset_top = 0; // Offset the legend up and down.
    int16_t legend_side_offset = 4; // Offset the legend off its anchor.
    Color legend_color = Color(255, 255, 255);
    Color legend_border_color = Color(200, 200, 200);
    double legend_border_width = 1;
    Anchor legend_anchor = Anchor::TOP_RIGHT;

    bool dont_draw_version_text = false;
};


class Figure {
public:
    Figure(GtkWidget* drawing_area, FigureStyle style = FigureStyle());
    virtual ~Figure() = default;

    void clear();
    void reset();
    void draw();
    void clear_axis();

    template<typename T, typename... Args>
    T* add_item(Args&&... args)
    {
        static_assert(
            std::is_base_of<PlotItem, T>::value,
            "T must inherit from PlotItem"
        );

        auto item = std::make_unique<T>(
            std::forward<Args>(args)...
        );

        T* ptr = item.get();

        {
            std::lock_guard<std::mutex> lock(data_mutex);
            plot_items.emplace_back(std::move(item));
        }

        return ptr;
    }

    LinePlot* plot(const std::vector<double>& x, const std::vector<double>& y, PlotStyle style = PlotStyle());
    LinePlot* plot(const std::vector<double>& y, PlotStyle style = PlotStyle());
    ScatterPlot* scatter(const std::vector<double> &x, const std::vector<double> &y, MarkerStyle style = MarkerStyle());
    ScatterPlot* scatter(const std::vector<double> &y, MarkerStyle style = MarkerStyle());
    PolygonPlot* polygon(const std::vector<double> &x, const std::vector<double> &y, ShapeStyle style = ShapeStyle());
    CirclePlot* circle(double x, double y, double radius, ShapeStyle style = ShapeStyle());

    Title* add_axis_title(AxisSide side, std::string title, TextStyle style);
    Title* add_plot_title(std::string title, TextStyle style = TextStyle());
    Title* add_x_title(std::string title, TextStyle style = TextStyle());
    Title* add_y_title(std::string title, TextStyle style = TextStyle());

    Text* add_text(double x, double y, std::string text, FigureTextStyle style = FigureTextStyle());

    GraphBounds get_manual_bounds(); // Returns the current status of the manual bounds.

    void legend(bool show = true);

    FigureStyle style;

    void export_image(std::string filename, uint32_t width, uint32_t height);

protected:

    cairo_surface_t* debug_image = nullptr;
    char version_label[64];

    GtkWidget* area;
    GraphBounds manual_bounds; // This is the bounds object which tracks bounds when they are set manually. If bounds are not set, this is not used.

    std::vector<std::unique_ptr<PlotItem>> plot_items;
    std::vector<std::unique_ptr<Axis>> axis_items;
    std::vector<std::unique_ptr<Text>> text_items; // We have a separate vector for this, to ensure text items get rendered on top.

    std::mutex data_mutex;
    std::mutex axis_mutex;
    std::mutex text_mutex;

    std::mutex draw_mutex;

    static gboolean on_draw(GtkWidget* widget, cairo_t* cr, gpointer user_data);
    static void on_size_allocate(GtkWidget* widget, GdkRectangle* allocation, gpointer user_data);
    
    void draw_no_data(cairo_t* cr, uint32_t width, uint32_t height);
    
    virtual double map_x(double x, double y, GraphViewport viewport);
    virtual double map_y(double x, double y, GraphViewport viewport);
    
    void draw_version_text(cairo_t* cr, uint32_t width, uint32_t height);
    void draw_axis(RenderContext& rc);
    void draw_legend(RenderContext& rc);
    void draw_text_items(RenderContext& rc);
    void draw_items(RenderContext& rc);

    virtual void draw_to_canvas(cairo_t* cr, uint32_t width, uint32_t height);

    MarginInformation get_margin_information();

    RenderContext get_render_context(cairo_t* cr, MarginInformation margins, GraphBounds context_bounds, uint32_t width, uint32_t height, bool square = false);
};

}

#endif // PLOTATO_FIGURE_HPP
