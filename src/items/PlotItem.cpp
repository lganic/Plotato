#include <Plotato/items/PlotItem.hpp>

namespace plotato {

bool PlotItem::check_is(PlotType check_type){
    return check_type == type();
}

}
