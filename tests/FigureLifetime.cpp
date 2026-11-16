#include <Plotato/Plotato.hpp>
#include <glib/gstdio.h>
#include <memory>

using namespace plotato;

namespace {

GtkWidget* make_area()
{
    // Keep a caller-owned reference so widget and figure lifetimes can be independent.
    GtkWidget* area = GTK_WIDGET(g_object_ref_sink(gtk_drawing_area_new()));
    gtk_widget_show(area);
    return area;
}

bool has_handlers(GtkWidget* area, Figure* figure)
{
    return g_signal_handler_find(area, G_SIGNAL_MATCH_DATA, 0, 0,
                                 nullptr, nullptr, figure) != 0;
}

gboolean count_draw(GtkWidget*, cairo_t*, gpointer data)
{
    ++*static_cast<int*>(data);
    return FALSE;
}

void count_resize(GtkWidget*, GtkAllocation*, gpointer data)
{
    ++*static_cast<int*>(data);
}

void count_destroy(GtkWidget*, gpointer data)
{
    ++*static_cast<int*>(data);
}

void emit_draw_and_resize(GtkWidget* area)
{
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 320, 240);
    cairo_t* cr = cairo_create(surface);
    gboolean handled = FALSE;
    g_signal_emit_by_name(area, "draw", cr, &handled);
    GtkAllocation allocation{0, 0, 320, 240};
    g_signal_emit_by_name(area, "size-allocate", &allocation);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

void figure_first()
{
    GtkWidget* area = make_area();
    int draws = 0, resizes = 0, destroys = 0;
    const gulong draw_id = g_signal_connect(area, "draw", G_CALLBACK(count_draw), &draws);
    const gulong resize_id = g_signal_connect(area, "size-allocate", G_CALLBACK(count_resize), &resizes);
    const gulong destroy_id = g_signal_connect(area, "destroy", G_CALLBACK(count_destroy), &destroys);

    std::unique_ptr<Figure> figure = std::make_unique<Graph>(area);
    Figure* identity = figure.get();
    g_assert_true(has_handlers(area, identity));
    figure->draw(); // A queued draw must also be safe after deleting the figure.
    figure.reset();

    g_assert_false(has_handlers(area, identity));
    g_assert_true(g_signal_handler_is_connected(area, draw_id));
    g_assert_true(g_signal_handler_is_connected(area, resize_id));
    g_assert_true(g_signal_handler_is_connected(area, destroy_id));
    g_assert_cmpint(destroys, ==, 0);
    emit_draw_and_resize(area);
    g_assert_cmpint(draws, ==, 1);
    g_assert_cmpint(resizes, ==, 1);

    // The caller can attach another graph to the same live widget.
    {
        PolarGraph replacement(area);
        replacement.plot({1.0, 2.0, 3.0});
        emit_draw_and_resize(area);
    }
    gtk_widget_destroy(area);
    g_assert_cmpint(destroys, ==, 1);
    g_object_unref(area);
}

void widget_first()
{
    GtkWidget* area = make_area();
    GtkWidget* weak_area = area;
    g_object_add_weak_pointer(G_OBJECT(area), reinterpret_cast<gpointer*>(&weak_area));
    std::unique_ptr<Figure> figure = std::make_unique<PolarGraph>(area);
    gtk_widget_destroy(area);
    g_object_unref(area);
    g_assert_null(weak_area); // Do not retain the caller's widget.
    figure->draw(); // No access to the finalized widget.

    GError* error = nullptr;
    gchar* directory = g_dir_make_tmp("plotato-lifetime-XXXXXX", &error);
    g_assert_no_error(error);
    gchar* filename = g_build_filename(directory, "export.png", nullptr);
    figure->plot({1.0, 2.0, 3.0});
    figure->export_image(filename, 320, 240);
    cairo_surface_t* exported = cairo_image_surface_create_from_png(filename);
    g_assert_cmpint(cairo_surface_status(exported), ==, CAIRO_STATUS_SUCCESS);
    g_assert_cmpint(cairo_image_surface_get_width(exported), ==, 320);
    g_assert_cmpint(cairo_image_surface_get_height(exported), ==, 240);
    cairo_surface_destroy(exported);
    g_unlink(filename);
    g_rmdir(directory);
    g_free(filename);
    g_free(directory);
    figure.reset();
}

void retained_widget()
{
    GtkWidget* area = make_area();
    auto figure = std::make_unique<Graph>(area);
    gtk_widget_destroy(area);
    g_assert_false(has_handlers(area, figure.get()));
    figure->draw();
    figure.reset();
    g_object_unref(area);
}

struct TrackedItem : LinePlot {
    int& destroyed;
    explicit TrackedItem(int& count) : LinePlot(std::vector<double>{1.0, 2.0}, PlotStyle()), destroyed(count) {}
    ~TrackedItem() override { ++destroyed; }
};

struct TrackedAxis : LinearAxis {
    int& destroyed;
    explicit TrackedAxis(int& count) : LinearAxis(BOTTOM), destroyed(count) {}
    ~TrackedAxis() override { ++destroyed; }
};

struct TrackedText : Text {
    int& destroyed;
    explicit TrackedText(int& count) : Text(0, 0, "test", FigureTextStyle()), destroyed(count) {}
    ~TrackedText() override { ++destroyed; }
};

template<class Base>
struct ResourceFigure : Base {
    using Base::Base;

    cairo_surface_t* hold_debug_image() { return cairo_surface_reference(this->debug_image); }

    void add_tracked_resources(int& items, int& axes, int& texts)
    {
        this->template add_item<TrackedItem>(items);
        this->axis_items.emplace_back(std::make_unique<TrackedAxis>(axes));
        this->text_items.emplace_back(std::make_unique<TrackedText>(texts));
    }
};

template<class Base>
void check_resources()
{
    GtkWidget* area = make_area();
    int items = 0, axes = 0, texts = 0;
    auto figure = std::make_unique<ResourceFigure<Base>>(area);
    figure->add_tracked_resources(items, axes, texts);
    cairo_surface_t* image = figure->hold_debug_image();
    g_assert_cmpint(cairo_surface_status(image), ==, CAIRO_STATUS_SUCCESS);
    g_assert_cmpuint(cairo_surface_get_reference_count(image), ==, 2);

    // Delete through the base class to exercise the virtual destructor pipeline.
    std::unique_ptr<Figure> base = std::move(figure);
    base.reset();
    g_assert_cmpuint(cairo_surface_get_reference_count(image), ==, 1);
    g_assert_cmpint(items, ==, 1);
    g_assert_cmpint(axes, ==, 1);
    g_assert_cmpint(texts, ==, 1);
    cairo_surface_destroy(image);
    gtk_widget_destroy(area);
    g_object_unref(area);
}

void resources()
{
    check_resources<Figure>();
    check_resources<Graph>();
    check_resources<PolarGraph>();
}

void shared_widget()
{
    GtkWidget* area = make_area();
    auto first = std::make_unique<Graph>(area);
    PolarGraph second(area);
    first.reset();
    g_assert_true(has_handlers(area, &second));
    gtk_widget_destroy(area);
    g_assert_false(has_handlers(area, &second));
    g_object_unref(area);
}

void repeated_lifetimes()
{
    for (int i = 0; i < 100; ++i) {
        GtkWidget* area = make_area();
        auto figure = std::make_unique<Graph>(area);
        if (i % 2 == 0) {
            figure.reset();
            emit_draw_and_resize(area);
            gtk_widget_destroy(area);
        } else {
            gtk_widget_destroy(area);
            figure->draw();
            figure.reset();
        }
        g_object_unref(area);
    }
}

}

int main(int argc, char** argv)
{
    g_test_init(&argc, &argv, nullptr);
    gtk_init(&argc, &argv);
    g_test_add_func("/figure/figure_first", figure_first);
    g_test_add_func("/figure/widget_first", widget_first);
    g_test_add_func("/figure/retained_widget", retained_widget);
    g_test_add_func("/figure/resources", resources);
    g_test_add_func("/figure/shared_widget", shared_widget);
    g_test_add_func("/figure/repeated_lifetimes", repeated_lifetimes);
    return g_test_run();
}
