#include "kee/global_assets.hpp"

namespace kee {

global_assets::global_assets() :
    shader_sdf_font(nullptr, "assets/shaders/sdf_text.fs"),
    shader_sdf_rect(nullptr, "assets/shaders/sdf_rect.fs"),
    sdf_rect_loc_color(shader_sdf_rect.GetLocation("color")),
    sdf_rect_loc_size(shader_sdf_rect.GetLocation("size")),
    sdf_rect_loc_roundness_size(shader_sdf_rect.GetLocation("roundnessSize")),
    sdf_rect_loc_outline_color(shader_sdf_rect.GetLocation("outlineColor")),
    sdf_rect_loc_outline_thickness(shader_sdf_rect.GetLocation("outlineThickness"))
{
    font.baseSize = 72;
    font.glyphCount = 95;
    font.glyphPadding = 0;
    font.recs = nullptr;

    const raylib::FileData font_file("assets/fonts/Montserrat-SemiBold.ttf");
    font.glyphs = LoadFontData(font_file.GetData(), font_file.GetBytesRead(), font.baseSize, nullptr, font.glyphCount, FontType::FONT_SDF);

    const raylib::Image font_atlas = GenImageFontAtlas(font.glyphs, &font.recs, font.glyphCount, font.baseSize, font.glyphPadding, 1);
    font.SetTexture(font_atlas.LoadTexture());
    font.GetTexture().SetFilter(TextureFilter::TEXTURE_FILTER_BILINEAR);

    raylib::Image img_empty(global_assets::texture_empty_size, global_assets::texture_empty_size, raylib::Color::Blank());
    texture_empty = raylib::Texture(img_empty);
}

} // namespace kee