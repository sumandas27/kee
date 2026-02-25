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
    );

    const std::string& get_string() const;
    float get_base_scale() const;

    void set_string(std::string_view new_str);
    void set_text_size_val(float val);

    const raylib::Font& font;

private:
    static constexpr float font_cap_height_multiplier_approx = 0.9f;

    void update_element(float dt) override;
    void render_element() const override;

    const bool has_clamped_width;
    const bool font_cap_height_only;

    std::string str;
    kee::ui::text_size str_size;
public:
    float str_render_size;
};

} // namespace ui
} // namespace kee