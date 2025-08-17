#include "kee/global_assets.hpp"

namespace kee {

global_assets::global_assets() :
    font_light(global_assets::gen_sdf_font("assets/fonts/Montserrat-Light.ttf")),
    font_regular(global_assets::gen_sdf_font("assets/fonts/Montserrat-Regular.ttf")),
    font_semi_bold(global_assets::gen_sdf_font("assets/fonts/Montserrat-SemiBold.ttf")),
    shader_sdf_font(nullptr, "assets/shaders/sdf_text.fs"),
    shader_sdf_rect(nullptr, "assets/shaders/sdf_rect.fs"),
    shader_sdf_triangle(nullptr, "assets/shaders/sdf_triangle.fs"),
    sdf_rect_loc_color(shader_sdf_rect.GetLocation("color")),
    sdf_rect_loc_size(shader_sdf_rect.GetLocation("size")),
    sdf_rect_loc_roundness_size(shader_sdf_rect.GetLocation("roundnessSize")),
    sdf_rect_loc_outline_color(shader_sdf_rect.GetLocation("outlineColor")),
    sdf_rect_loc_outline_thickness(shader_sdf_rect.GetLocation("outlineThickness")),
    sdf_triangle_loc_color(shader_sdf_triangle.GetLocation("color")),
    sdf_triangle_loc_size(shader_sdf_triangle.GetLocation("size")),
    sdf_triangle_loc_p0(shader_sdf_triangle.GetLocation("p0")),
    sdf_triangle_loc_p1(shader_sdf_triangle.GetLocation("p1")),
    sdf_triangle_loc_p2(shader_sdf_triangle.GetLocation("p2"))
{
    const raylib::Image img_empty(global_assets::texture_empty_size, global_assets::texture_empty_size, raylib::Color::Blank());
    texture_empty = raylib::Texture(img_empty);
}

raylib::Font global_assets::gen_sdf_font(const std::filesystem::path& font_path)
{
    raylib::Font res;
    res.baseSize = 72;
    res.glyphCount = 95;
    res.glyphPadding = 0;
    res.recs = nullptr;

    const raylib::FileData font_file(font_path.string());
    res.glyphs = LoadFontData(font_file.GetData(), font_file.GetBytesRead(), res.baseSize, nullptr, res.glyphCount, FontType::FONT_SDF);

    const raylib::Image font_atlas = GenImageFontAtlas(res.glyphs, &res.recs, res.glyphCount, res.baseSize, res.glyphPadding, 1);
    res.SetTexture(font_atlas.LoadTexture());
    res.GetTexture().SetFilter(TextureFilter::TEXTURE_FILTER_BILINEAR);

    return res;
}

} // namespace kee