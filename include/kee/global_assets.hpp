#pragma once

/**
 * Disabling MSVC warnings on raylib's source code.
 */
#ifdef _MSC_VER
    #pragma warning(disable: 4458)
#endif
#include <raylib-cpp.hpp>
#ifdef _MSC_VER
    #pragma warning(default: 4458)
#endif

namespace kee {

class global_assets
{
public:
    static constexpr int texture_empty_size = 1;

    global_assets();

    raylib::Font font;
    raylib::Shader shader_sdf_font;

    raylib::Shader shader_sdf_rect;
    raylib::Texture texture_empty;

    const int sdf_rect_loc_color;
    const int sdf_rect_loc_size;
    const int sdf_rect_loc_roundness_size;
    const int sdf_rect_loc_outline_color;
    const int sdf_rect_loc_outline_thickness;
};

} // namespace kee