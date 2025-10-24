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
        const std::optional<raylib::Color>& color, 
        kee::pos p_x, 
        kee::pos p_y, 
        kee::ui::text_size p_str_size,
        bool centered,
        const raylib::Font& font,
        std::string_view p_string,
        bool font_cap_height_only
    );

    const std::string& get_string() const;

    void set_string(std::string_view new_str);
    void set_scale(float new_scale);

private:
    void render_element() const override;

    void update_dims(
        std::optional<std::string_view> new_str, 
        std::optional<kee::ui::text_size> new_str_size, 
        std::optional<float> new_scale
    );

    const raylib::Font& font;

    static constexpr float font_cap_height_multiplier_approx = 0.9f;
    const bool font_cap_height_only;

    std::string str;
    float str_size;
    float scale;
};

} // namespace ui
} // namespace kee