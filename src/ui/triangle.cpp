#include "kee/ui/triangle.hpp"

namespace kee {
namespace ui {

triangle::triangle(
    const kee::ui::required& reqs, 
    const std::optional<raylib::Color>& color, 
    kee::pos x,
    kee::pos y,
    const std::variant<kee::dims, kee::border>& dims,
    bool centered,
    const raylib::Vector2& p0,
    const raylib::Vector2& p1,
    const raylib::Vector2& p2
) :
    kee::ui::base(reqs, x, y, dims, centered),
    p0(p0),
    p1(p1),
    p2(p2)
{ 
    set_opt_color(color);
}

void triangle::render_element() const
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

    const std::array<float, 2> uniform_p0 = { p0.x, p0.y };
    const std::array<float, 2> uniform_p1 = { p1.x, p1.y };
    const std::array<float, 2> uniform_p2 = { p2.x, p2.y };

    reqs.assets.shader_sdf_triangle.SetValue(reqs.assets.sdf_triangle_loc_color, uniform_color.data(), ShaderUniformDataType::SHADER_UNIFORM_VEC4);
    reqs.assets.shader_sdf_triangle.SetValue(reqs.assets.sdf_triangle_loc_size, uniform_size.data(), ShaderUniformDataType::SHADER_UNIFORM_VEC2);
    reqs.assets.shader_sdf_triangle.SetValue(reqs.assets.sdf_triangle_loc_p0, uniform_p0.data(), ShaderUniformDataType::SHADER_UNIFORM_VEC2);
    reqs.assets.shader_sdf_triangle.SetValue(reqs.assets.sdf_triangle_loc_p1, uniform_p1.data(), ShaderUniformDataType::SHADER_UNIFORM_VEC2);
    reqs.assets.shader_sdf_triangle.SetValue(reqs.assets.sdf_triangle_loc_p2, uniform_p2.data(), ShaderUniformDataType::SHADER_UNIFORM_VEC2);

    reqs.assets.shader_sdf_triangle.BeginMode();
    reqs.assets.texture_empty.Draw(src_rect, raw_rect);
    reqs.assets.shader_sdf_triangle.EndMode();
}

} // namespace ui
} // namespace kee