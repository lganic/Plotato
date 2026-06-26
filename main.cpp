#include <gtk/gtk.h>
#include <cmath>
#include <memory>
#include <vector>
#include "Graph.hpp"

int main(int argc, char** argv)
{
    gtk_init(&argc, &argv);

    GtkBuilder* builder = gtk_builder_new_from_file("graph_test.glade");

    GtkWidget* window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    GtkWidget* drawing_area = GTK_WIDGET(gtk_builder_get_object(builder, "graph_area"));

    if (!window || !drawing_area) {
        g_printerr("Failed to load window or drawing area from Glade file.\n");
        return 1;
    }

    GraphBounds bounds {
        0.0, 10.0,
        -1.2, 1.2
    };

    Graph graph(drawing_area, bounds);

    std::vector<double> x;
    std::vector<double> y;

    for (int i = 0; i <= 500; i++) {
        double t = i * 10.0 / 500.0;
        x.push_back(t);
        y.push_back(std::sin(t));
    }

    // graph.plot(x, y);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    g_object_unref(builder);
    return 0;
}