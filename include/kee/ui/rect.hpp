#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {

class rect_outline
{
public:
    enum class type;

    rect_outline(rect_outline::type rect_outline_type, float val, const std::optional<raylib::Color>& opt_color);

    const std::optional<raylib::Color> opt_color;
    const rect_outline::type rect_outline_type;
    const float val;
};

enum class rect_outline::type
{
    abs,
    rel_w,
    rel_w_parent,
    rel_h,
    rel_h_parent
};

class rect_roundness
{
public:
    enum class type;
    enum class size_effect;

    rect_roundness(rect_roundness::type rect_roundness_type, float val, std::optional<rect_roundness::size_effect> rect_size_effect);

    const rect_roundness::type rect_roundness_type;
    const float val;
    const std::optional<rect_roundness::size_effect> rect_size_effect;
};

enum class rect_roundness::type
{
    abs,
    rel_w,
    rel_h
};

enum class rect_roundness::size_effect
{
    extend_w,
    extend_h
};

class rect final : public kee::ui::base
{
public:
    rect(
        const kee::ui::base::required& reqs, 
        const std::optional<raylib::Color>& color, 
        kee::pos x,
        kee::pos y,
        const std::variant<kee::dims, kee::border>& dims, 
        std::optional<kee::ui::rect_outline> border,
        std::optional<kee::ui::rect_roundness> roundness,
        const kee::ui::common& common
    );

    raylib::Rectangle get_raw_rect() const override;

private:
    void render_element_behind_children() const override;

    float get_roundness() const;

    const std::optional<kee::ui::rect_outline> border;
    const std::optional<kee::ui::rect_roundness> roundness;
};

} // namespace ui
} // namespace kee