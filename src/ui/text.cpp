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
            clamped_width.has_value() ? clamped_width.value() :dim(dim::type::abs, 0),
            dim(p_str_size.text_size_type == text_size::type::abs ? dim::type::abs : dim::type::rel, p_str_size.val)
        ),
        centered
    ),
    font(font),
    has_clamped_width(clamped_width.has_value()),
    font_cap_height_only(font_cap_height_only),
    str(p_string),
    str_size(p_str_size)
{
    color = color_param;

    /**
     * Sets strings and dimensions at construction time.
     */
    update_element(0.f);
}

const std::string& text::get_string() const
{
    return str;
}

float text::get_base_scale() const
{
    return str_render_size / font.baseSize;
}

void text::set_string(std::string_view new_str)
{
    str = new_str;
}

void text::set_text_size_val(float val)
{
    std::get<kee::dims>(dimensions).h.val = val;
}

void text::update_element([[maybe_unused]] float dt)
{
    const raylib::Rectangle raw_rect = get_raw_rect();
    str_render_size = raw_rect.height;

    raylib::Vector2 ui_text_dims = font.MeasureText(str.data(), str_render_size, 0.0f);
    if (has_clamped_width)
    {
        float clamped_width_val = raw_rect.width;

        std::size_t str_end_char = str.size();
        while (str_end_char > 0 && ui_text_dims.x > clamped_width_val)
        {
            str = str.substr(0, str_end_char) + "...";

            ui_text_dims = font.MeasureText(str.data(), str_render_size, 0.0f);
            str_end_char--;
        }
    }
    else
    {
        auto& [w, h] = std::get<kee::dims>(dimensions);
        w.val = ui_text_dims.x;
    }
}

void text::render_element() const
{
    raylib::Rectangle raw_rect = get_raw_rect();
    if (font_cap_height_only)
        raw_rect.y += raw_rect.height * (1.0f - font_cap_height_multiplier_approx);

    assets.shader_sdf_font.BeginMode();
    font.DrawText(str.c_str(), raw_rect.GetPosition(), str_render_size, 0.0f, color.raylib());
    assets.shader_sdf_font.EndMode();
}

} // namespace ui
} // namespace kee