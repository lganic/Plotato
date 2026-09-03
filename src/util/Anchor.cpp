#include <Plotato/util/Anchor.hpp>
#include <utility>

namespace plotato {
    
std::pair<double, double> get_offset(double item_width, double item_height, Anchor anchor) {

    double wd2 = item_width / 2;
    double hd2 = item_height / 2;

    switch (anchor)
    {

    case Anchor::TOP_LEFT:
        return {wd2, hd2};
    case Anchor::TOP_CENTER:
        return {0, hd2};
    case Anchor::TOP_RIGHT:
        return {-wd2, hd2};
    case Anchor::MIDDLE_LEFT:
        return {wd2, 0};
    case Anchor::MIDDLE_CENTER:
        return {0, 0};
    case Anchor::MIDDLE_RIGHT:
        return {-wd2, 0};
    case Anchor::BOTTOM_LEFT:
        return {wd2, -hd2};
    case Anchor::BOTTOM_CENTER:
        return {0, -hd2};
    case Anchor::BOTTOM_RIGHT:
        return {-wd2, -hd2};
    default:
        return {0, 0};
    }


}

}

