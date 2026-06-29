#include <Plotato/util/StyleStructs.hpp>
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

}

