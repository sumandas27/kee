#include "kee/ui/rect.hpp"

#include <array>

namespace kee {
namespace ui {

rect_outline::rect_outline(rect_outline::type rect_outline_type, float val, const std::optional<raylib::Color>& opt_color) :
    opt_color(opt_color),
    rect_outline_type(rect_outline_type),
    val(val)
{ }

rect_roundness::rect_roundness(rect_roundness::type rect_roundness_type, float val) :
    rect_roundness_type(rect_roundness_type),
    val(val)
{ }

rect::rect(
    const kee::ui::base::required& reqs, 
    const std::optional<raylib::Color>& color, 
    kee::pos x, 
    kee::pos y, 
    const std::variant<kee::dims, kee::border>& dims, 
    std::optional<kee::ui::rect_outline> border,
    std::optional<kee::ui::rect_roundness> roundness,
    const kee::ui::common& common
) :
    kee::ui::base(reqs, x, y, dims, common),
    border(border),
    roundness(roundness)
{
    set_opt_color(color);
}

void rect::render_element_behind_children() const 
{
    static const raylib::Rectangle src_rect(0, 0, global_assets::texture_empty_size, global_assets::texture_empty_size);

    const raylib::Rectangle raw_rect = get_raw_rect();
    const raylib::Color raw_color = get_color_from_opt(get_opt_color());

    const std::array<float, 2> uniform_size = { raw_rect.width, raw_rect.height };
    const std::array<float, 4> uniform_color = { 
        static_cast<float>(raw_color.r) / 255.0f,
        static_cast<float>(raw_color.g) / 255.0f,
        static_cast<float>(raw_color.b) / 255.0f,
        static_cast<float>(raw_color.a) / 255.0f
    };

    float uniform_roundness_size;
    if (roundness.has_value())
    {
        switch (roundness.value().rect_roundness_type)
        {
        case rect_roundness::type::abs:
            uniform_roundness_size = roundness.value().val;
            break;
        case rect_roundness::type::rel_w:
            uniform_roundness_size = raw_rect.width * roundness.value().val;
            break;
        case rect_roundness::type::rel_h:
            uniform_roundness_size = raw_rect.height * roundness.value().val;
            break;
        default:
            std::unreachable();
        }
    }
    else
        uniform_roundness_size = 0;

    float uniform_outline_thickness = 0;
    std::array<float, 4> uniform_outline_color = { 0.0f, 0.0f, 0.0f, 0.0f };    
    if (border.has_value())
    {
        switch (border.value().rect_outline_type)
        {
        case rect_outline::type::abs:
            uniform_outline_thickness = border.value().val;
            break;
        case rect_outline::type::rel_w:
            uniform_outline_thickness = raw_rect.width * border.value().val;
            break;
        case rect_outline::type::rel_w_parent:
            uniform_outline_thickness = get_raw_rect_parent().width * border.value().val;
            break;
        case rect_outline::type::rel_h:
            uniform_outline_thickness = raw_rect.height * border.value().val;
            break;
        case rect_outline::type::rel_h_parent:
            uniform_outline_thickness = get_raw_rect_parent().height * border.value().val;
            break;
        default:
            std::unreachable();
        }

        const raylib::Color outline_color = get_color_from_opt(border.value().opt_color);
        uniform_outline_color = { 
            static_cast<float>(outline_color.r) / 255.0f,
            static_cast<float>(outline_color.g) / 255.0f,
            static_cast<float>(outline_color.b) / 255.0f,
            static_cast<float>(outline_color.a) / 255.0f
        };
    }

    assets.shader_sdf_rect.SetValue(assets.sdf_rect_loc_color, uniform_color.data(), ShaderUniformDataType::SHADER_UNIFORM_VEC4);
    assets.shader_sdf_rect.SetValue(assets.sdf_rect_loc_size, uniform_size.data(), ShaderUniformDataType::SHADER_UNIFORM_VEC2);
    assets.shader_sdf_rect.SetValue(assets.sdf_rect_loc_roundness_size, &uniform_roundness_size, ShaderUniformDataType::SHADER_UNIFORM_FLOAT);
    assets.shader_sdf_rect.SetValue(assets.sdf_rect_loc_outline_color, uniform_outline_color.data(), ShaderUniformDataType::SHADER_UNIFORM_VEC4);
    assets.shader_sdf_rect.SetValue(assets.sdf_rect_loc_outline_thickness, &uniform_outline_thickness, ShaderUniformDataType::SHADER_UNIFORM_FLOAT);

    assets.shader_sdf_rect.BeginMode();
    assets.texture_empty.Draw(src_rect, raw_rect);
    assets.shader_sdf_rect.EndMode();
}

} // namespace ui
} // namespace kee