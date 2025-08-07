#include "kee/ui/text.hpp"

namespace kee {
namespace ui {

text_size::text_size(text_size::type text_size_type, float val) :
    text_size_type(text_size_type),
    val(val)
{ }

text::text(
    kee::ui::base& parent, 
    const std::optional<raylib::Color>& color, 
    kee::ui::pos p_x, 
    kee::ui::pos p_y, 
    kee::ui::text_size p_str_size, 
    std::string_view p_string, 
    bool centered,
    std::optional<int> z_order,
    bool children_z_order_enabled
) :
    kee::ui::base(
        parent, p_x, p_y, 
        dims(
            dim(dim::type::abs, 0), 
            dim(dim::type::abs, 0)
        ), 
        centered, z_order, children_z_order_enabled
    )
{ 
    set_color(color);
    update_dims(p_string, p_str_size);
}

void text::render_element() const
{
    text::global::get_font().DrawText(str.c_str(), get_raw_rect().GetPosition(), str_size, 0.0f, get_color());
}

void text::update_dims(std::optional<std::string_view> new_str, std::optional<kee::ui::text_size> new_str_size)
{
    if (new_str.has_value())
        str = new_str.value();
    
    if (new_str_size.has_value())
        switch (new_str_size.value().text_size_type)
        {
        case text_size::type::abs:
            str_size = new_str_size.value().val;
            break;
        case text_size::type::rel_h:
            str_size = get_raw_rect_parent().height * new_str_size.value().val;
            break;
        }

    const raylib::Vector2 ui_text_dims = text::global::get_font().MeasureText(str.data(), str_size, 0.0f);
    auto& [w, h] = std::get<kee::ui::dims>(dimensions);
    w.val = ui_text_dims.x;
    h.val = ui_text_dims.y;
}

const raylib::Font& text::global::get_font()
{
    return singleton().font;
}

text::global& text::global::singleton()
{
    static text::global instance;
    return instance;
}

text::global::global()
{ 
    font.baseSize = 72;
    font.glyphCount = 95;
    font.glyphPadding = 0;
    font.recs = nullptr;

    const raylib::FileData font_file("assets/fonts/Montserrat-Light.ttf");
    font.glyphs = LoadFontData(font_file.GetData(), font_file.GetBytesRead(), font.baseSize, nullptr, font.glyphCount, FontType::FONT_SDF);

    const raylib::Image font_atlas = GenImageFontAtlas(font.glyphs, &font.recs, font.glyphCount, font.baseSize, font.glyphPadding, 1);
    font.SetTexture(font_atlas.LoadTexture());
    font.GetTexture().SetFilter(TextureFilter::TEXTURE_FILTER_BILINEAR);
}

} // namespace ui
} // namespace kee