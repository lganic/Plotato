# Plotato
<p align="center">
  <img src="https://github.com/lganic/Plotato/blob/main/static/Icon.png?raw=true">
</p>

Plotato is a plotting library designed from the ground up to operate in a GTK3 environment, for a diverse set of plotting applications, including live plotting of values. It is written in C++, for ease of use and performance. 

Below, you can see a minimal example of how Plotato can be imported, initialized and used to plot some sample data.

```cpp
#include <gtk/gtk.h>
#include <Plotato/Plotato.hpp>
#include <vector>

int main(int argc, char* argv[])
{
    gtk_init(&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL); // Make a new window.

    GtkWidget* drawing_area = gtk_drawing_area_new(); // Make a drawing area for the graph.
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    // Declare a new graph in the new drawing area.
    plotato::Graph g(drawing_area);

    // Make some data to plot.
    std::vector<double> x_data {1,2,3,4,5};
    std::vector<double> y_data {1,2,0,-1,0};

    // Plot the data on the graph.
    g.plot(x_data, y_data);


    // Some more gtk stuff for our window.
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);

    // Show the window, which will automatically draw the plot.
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
```

This minimal example produces a window as seen below. 

<p align="center">
  <img src="https://github.com/lganic/Plotato/blob/main/static/Example.png?raw=true">
</p>

This window can be resized, withought any additional changes to the code. Plotato automatically handles the resizing events, re-drawing the graph area for the space given. 

<p align="center">
  <img src="https://github.com/lganic/Plotato/blob/main/static/Example-Resize.gif?raw=true">
</p>

Axis can optionally be added by calling the "add_linear_axis" member function on the graph:

```cpp
g.add_linear_axis(plotato::BOTTOM);
g.add_linear_axis(plotato::LEFT);
```

<p align="center">
  <img src="https://github.com/lganic/Plotato/blob/main/static/Example-Axis.png?raw=true">
</p>

Similarly, titles can be added to each axis, by calling the corresponding function.

```cpp
g.add_x_title("X Axis");
g.add_y_title("Y Axis");
g.add_plot_title("Testing Plot");
```

Setting the style of the plot titles can be done either while calling the add title function by passing a text style object, or later by modifying the text style attribute:

```cpp
auto title = g.add_plot_title("Testing Plot");

title->text_style.font_size = 20;
title->text_style.text_color = plotato::Color(255, 0, 0);
```

<p align="center">
  <img src="https://github.com/lganic/Plotato/blob/main/static/Example-Title.png?raw=true">
</p>

To make a live plot, we simply need to clear the old contents of the graph, add the new contents, and then call the draw() member function of the graph. Below, we will use a callback to continually update the contents of the graph. 

```cpp
#include <gtk/gtk.h>

#include <cmath>
#include <vector>

#include <Plotato/Plotato.hpp>

struct CallbackData
{
    plotato::Graph* graph = nullptr;

    double offset = 0.0;
    double offset_step = 0.01;

    std::vector<double> x;
    std::vector<double> y;
};

static gboolean update_graph(gpointer user_data)
{
    auto* data = static_cast<CallbackData*>(user_data);

    // This is just making the live data.
    constexpr std::size_t sample_count = 501;

    data->x.clear();
    data->y.clear();

    for (std::size_t i = 0; i < sample_count; ++i)
    {
        const double t = static_cast<double>(i) / static_cast<double>(sample_count - 1);

        const double x = t + data->offset;
        const double value = std::sin(5.0 * std::sin(0.01 * x) * x);

        data->x.push_back(x);
        data->y.push_back(1.0 * value);
    }

    // This is where we actually get to the plot stuff.
    data->graph->clear(); // Clear the old plot data.

    data->graph->plot(data->x, data->y); // Plot the new data.

    data->graph->draw(); // Draw the graph.

    data->offset += data->offset_step;

    return G_SOURCE_CONTINUE;
}

int main(int argc, char** argv)
{
    gtk_init(&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL); // Make a new window.

    GtkWidget* drawing_area = gtk_drawing_area_new(); // Make a drawing area for the graph.
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    plotato::Graph graph(drawing_area);

    // Setting bounds manually for y axis.
    plotato::GraphBounds bounds;
    bounds.ymax = 1.5;
    bounds.ymin = -1.5;

    graph.set_bounds(bounds);

    graph.add_linear_axis(plotato::BOTTOM);
    graph.add_linear_axis(plotato::LEFT);

    auto title = graph.add_plot_title("Testing Plot");
    title->text_style.font_size = 20;

    graph.add_x_title("X Title");
    graph.add_y_title("Y Title");

    CallbackData callback_data;
    callback_data.graph = &graph;
    callback_data.offset_step = 0.01;

    constexpr std::size_t sample_count = 501;

    callback_data.x.reserve(sample_count);
    callback_data.y.reserve(sample_count);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);

    g_timeout_add(10, update_graph, &callback_data);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
```

<p align="center">
  <img src="https://github.com/lganic/Plotato/blob/main/static/Example-Live.gif?raw=true">
</p>