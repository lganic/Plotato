#ifndef PLOTATO_ANCHOR_HPP
#define PLOTATO_ANCHOR_HPP

#include <utility>

namespace plotato {

enum class Anchor {
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,
    MIDDLE_LEFT,
    MIDDLE_CENTER,
    MIDDLE_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT
};

std::pair<double, double> get_offset(double item_width, double item_height, Anchor anchor);

}

#endif // PLOTATO_ANCHOR_HPP