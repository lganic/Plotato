#ifndef PLOTATO_STYLESTRUCTS_HPP
#define PLOTATO_STYLESTRUCTS_HPP

#include <cstdint>
#include <gtk/gtk.h>
#include <cstdint>
#include <string>

namespace plotato {

struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    constexpr Color (uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}

    void to_cairo_source(cairo_t* cr);
};

struct TextStyle {
    Color text_color;
    uint16_t font_size = 12;

    std::string font = "Sans";

    void to_cairo_source(cairo_t* cr);
};

struct MarkerStyle {
    char type;
    Color fill;
    
    Color outline = Color(0, 0, 0, 0);
    double outline_width = 2;
};

struct PlotStyle {
    Color line_color = Color(0, 0, 255);
    double line_width = 2;

    std::string name = "Plot";

    MarkerStyle marker;

    void to_cairo_source(cairo_t* cr);
};

uint32_t get_approx_vertical_size(uint32_t font_size);
uint32_t get_approx_drop_size(uint32_t font_size);
uint32_t get_approx_horizontal_size(uint32_t font_size);

}

#endif // PLOTATO_STYLESTRUCTS_HPP
