#include <Plotato/util/StyleStructs.hpp>
#include <cstdint>
#include <gtk/gtk.h>

namespace plotato {

void Color::to_cairo_source(cairo_t* cr){
    
    cairo_set_source_rgba(cr, 
        r / 255.0,
        g / 255.0,
        b / 255.0,
        a / 255.0
    );

}

void TextStyle::to_cairo_source(cairo_t* cr){

    text_color.to_cairo_source(cr);

    cairo_select_font_face(cr, font.c_str(), CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

    cairo_set_font_size(cr, font_size);
}

void PlotStyle::to_cairo_source(cairo_t* cr){

    line_color.to_cairo_source(cr);
}


uint32_t get_approx_vertical_size(uint32_t font_size) {
    return font_size * 0.7;
}

uint32_t get_approx_drop_size(uint32_t font_size) {
    return font_size * 0.4;
}

uint32_t get_approx_horizontal_size(uint32_t font_size) {
    return get_approx_vertical_size(font_size) * 4; // Very rough approximation!
}

}