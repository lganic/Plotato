#ifndef PLOTATO_STYLESTRUCTS_HPP
#define PLOTATO_STYLESTRUCTS_HPP

#include <cstdint>
#include <gtk/gtk.h>

namespace plotato {

struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    constexpr Color (uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}

    void to_cairo_source(cairo_t* cr);
};

}

#endif // PLOTATO_STYLESTRUCTS_HPP
