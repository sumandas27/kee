#include "kee/scene.hpp"

#include <iostream>

namespace kee {

scene::scene(const raylib::Vector2& window_dim)
{
    const float size_diff = window_dim.x * 0.1f;
    const raylib::Vector2 rect_keys_raw_size(window_dim.x - 2 * size_diff, window_dim.y - 2 * size_diff);
    const raylib::Vector2 rect_keys_aspect(16, 9);

    if (rect_keys_aspect.x * rect_keys_raw_size.y >= rect_keys_aspect.y * rect_keys_raw_size.x)
    {
        rect_keys.height = rect_keys_raw_size.y;
        rect_keys.width = rect_keys.height * rect_keys_aspect.x / rect_keys_aspect.y;
    }
    else
    {
        rect_keys.width = rect_keys_raw_size.x;
        rect_keys.height = rect_keys.width * rect_keys_aspect.y / rect_keys_aspect.x;
    }

    rect_keys.x = (window_dim.x - rect_keys.width) / 2;
    rect_keys.y = (window_dim.y - rect_keys.height) / 2;
}

void scene::render() const
{
    rect_keys.Draw(DARKGRAY);
}

}