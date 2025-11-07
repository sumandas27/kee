#include "kee/ui_common.hpp"

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

transition_base::transition_base() :
    timer(0.0f)
{ }

void transition_base::update(float dt)
{
    if (timer > 0.0f)
    {
        timer -= dt;
        if (timer <= 0.0f)
            timer = 0.0f;
    }
}

const kee::color color::dark_orange = kee::color(255, 140, 0);
const kee::color color::green = kee::color(0, 255, 0);
const kee::color color::red = kee::color(255, 0, 0);
const kee::color color::white = kee::color(255, 255, 255);

raylib::Color color::to_color() const
{
    return raylib::Color(
        static_cast<unsigned char>(r),
        static_cast<unsigned char>(g),
        static_cast<unsigned char>(b),
        static_cast<unsigned char>(a)
    );
}

kee::color color::operator*(float scalar) const
{
    return kee::color(r * scalar, g * scalar, b * scalar, a * scalar);
}

kee::color color::operator+(const kee::color& other) const
{
    return kee::color(r + other.r, g + other.g, b + other.b, a + other.a);
}

key_pos_data::key_pos_data(int raylib_key, const raylib::Vector2& relative_pos) :
    raylib_key(raylib_key),
    relative_pos(relative_pos)
{ }

} // namespace kee