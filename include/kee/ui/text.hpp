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
        kee::ui::base& parent, 
        const std::optional<raylib::Color>& color, 
        kee::ui::pos p_x, 
        kee::ui::pos p_y, 
        kee::ui::text_size p_str_size, 
        std::string_view p_string, 
        bool centered,
        std::optional<int> z_order,
        bool children_z_order_enabled
    );

private:
    class global;

    void render_element() const override;

    void update_dims(std::optional<std::string_view> new_str, std::optional<kee::ui::text_size> new_str_size);

    std::string str;
    float str_size;
};

class text::global
{
public:
    static const raylib::Font& get_font();

private:
    static text::global& singleton();

    global();

    raylib::Font font;
};

} // namespace ui
} // namespace kee