#include "kee/transition.hpp"

namespace kee {

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

} // namespace kee