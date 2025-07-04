#pragma once

/**
 * Disabling a bunch of warnings on raylib's source code.
 */
#ifdef _MSC_VER
    #pragma warning(disable: 4458)
#endif
#include "raylib-cpp.hpp"
#ifdef _MSC_VER
    #pragma warning(default: 4458)
#endif

namespace kee {

class scene
{
public:
    scene(const raylib::Vector2& window_dim);

    void render() const;

private:
    raylib::Rectangle rect_keys;
};

} // namespace kee