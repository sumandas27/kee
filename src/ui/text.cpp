#include "kee/ui/text.hpp"

namespace kee {
namespace ui {

text_size::text_size(text_size::type text_size_type, float val) :
    text_size_type(text_size_type),
    val(val)
{ }

text::text(
    const kee::ui::required& reqs, 
    const kee::color& color_param, 
    const kee::pos& p_x, 
    const kee::pos& p_y,
    const kee::ui::text_size& p_str_size,
    const std::optional<kee::dim>& clamped_width,
    bool centered,
    const raylib::Font& font,
    std::string_view p_string,
    bool font_cap_height_only
) :
    kee::ui::base(
        reqs, p_x, p_y, 
        dims(
            dim(dim::type::abs, 0), 
            dim(dim::type::abs, 0)
        ), 
        centered
    ),
    font(font),
    clamped_width(clamped_width),
    font_cap_height_only(font_cap_height_only)
{
    color = color_param;
    update_dims(p_string, p_str_size, 1.0f);
}

const std::string& text::get_string() const
{
    return str;
}

float text::get_base_scale() const
{
    return str_size * scale / font.baseSize;
}

void text::set_string(std::string_view new_str)
{
    update_dims(new_str, std::nullopt, std::nullopt);
}

void text::set_scale(float new_scale)
{
    update_dims(std::nullopt, std::nullopt, new_scale);
}

void text::render_element() const
{
    raylib::Rectangle raw_rect = get_raw_rect();
    if (font_cap_height_only)
        raw_rect.y += raw_rect.height * (1.0f - font_cap_height_multiplier_approx);

    assets.shader_sdf_font.BeginMode();
    font.DrawText(str.c_str(), raw_rect.GetPosition(), str_size * scale, 0.0f, color.raylib());
    assets.shader_sdf_font.EndMode();
}

void text::update_dims(
    std::optional<std::string_view> new_str, 
    std::optional<kee::ui::text_size> new_str_size, 
    std::optional<float> new_scale
) {
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

    if (new_scale.has_value())
        scale = new_scale.value();

    raylib::Vector2 ui_text_dims = font.MeasureText(str.data(), str_size * scale, 0.0f);
    if (clamped_width.has_value())
    {
        float clamped_width_val;
        switch (clamped_width.value().dim_type)
        {
        case kee::dim::type::abs:
            clamped_width_val = clamped_width.value().val;
            break;
        case kee::dim::type::rel:
            clamped_width_val = get_raw_rect_parent().width * clamped_width.value().val;
            break;
        case kee::dim::type::aspect:
            clamped_width_val = ui_text_dims.y * clamped_width.value().val; 
            break;
        default:
            std::unreachable();
        }

        std::size_t str_end_char = str.size();
        while (str_end_char > 0 && ui_text_dims.x > clamped_width_val)
        {
            str = str.substr(0, str_end_char) + "...";

            ui_text_dims = font.MeasureText(str.data(), str_size * scale, 0.0f);
            str_end_char--;
        }
    }

    auto& [w, h] = std::get<kee::dims>(dimensions);
    w.val = ui_text_dims.x;
    h.val = ui_text_dims.y;
}

} // namespace ui
} // namespace kee