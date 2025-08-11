#pragma once

#include <string_view>

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {

class text_size
{
public:
    enum class type;

    text_size(text_size::type text_size_type, float val);

    text_size::type text_size_type;
    float val;
};

enum class text_size::type
{
    abs,
    rel_h
};

class text final : public kee::ui::base
{
public:
    text(
        const kee::ui::base& parent, 
        const std::optional<raylib::Color>& color, 
        kee::pos p_x, 
        kee::pos p_y, 
        kee::ui::text_size p_str_size, 
        std::string_view p_string,
        bool font_cap_height_only,
        const kee::ui::common& common
    );

    void set_string(std::string_view new_str);
    void set_scale(float new_scale);

private:
    class global;

    void render_element() const override;

    void update_dims(std::optional<std::string_view> new_str, std::optional<kee::ui::text_size> new_str_size, std::optional<float> new_scale);

    static constexpr float font_cap_height_multiplier_approx = 0.9f;
    const bool font_cap_height_only;

    std::string str;
    float str_size;
    float scale;
};

class text::global
{
public:
    static const raylib::Font& get_font();
    static raylib::Shader& get_sdf_shader();

private:
    static text::global& singleton();

    global();

    raylib::Font font;
    raylib::Shader sdf_shader;
};

} // namespace ui
} // namespace kee