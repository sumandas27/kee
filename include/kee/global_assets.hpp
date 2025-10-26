#pragma once

#include <filesystem>

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
    
class image_texture
{
public:
    image_texture(const std::filesystem::path& img_path);

    raylib::Texture texture;
};

class global_assets
{
public:
    static constexpr int texture_empty_size = 1;

    global_assets();

    const raylib::Font font_light;
    const raylib::Font font_regular;
    const raylib::Font font_semi_bold;

    raylib::Shader shader_sdf_font;
    raylib::Shader shader_sdf_rect;
    raylib::Shader shader_sdf_triangle;
    raylib::Texture texture_empty;

    const int sdf_rect_loc_color;
    const int sdf_rect_loc_size;
    const int sdf_rect_loc_roundness_size;
    const int sdf_rect_loc_outline_color;
    const int sdf_rect_loc_outline_thickness;

    const int sdf_triangle_loc_color;
    const int sdf_triangle_loc_size;
    const int sdf_triangle_loc_p0;
    const int sdf_triangle_loc_p1;
    const int sdf_triangle_loc_p2;

    const kee::image_texture play_png;
    const kee::image_texture directory_png;

private:
    static raylib::Font gen_sdf_font(const std::filesystem::path& font_path);
};

} // namespace kee