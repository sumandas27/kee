#include "kee/geometry.hpp"

namespace kee {

pos::pos(pos::type pos_type, float val) :
    pos_type(pos_type),
    val(val)
{ }

dim::dim(dim::type dim_type, float val) :
    dim_type(dim_type),
    val(val)
{ }

dims::dims(dim w, dim h) :
    w(w),
    h(h)
{ }

border::border(border::type border_type, float val) :
    border_type(border_type),
    val(val)
{ }

} // namespace kee