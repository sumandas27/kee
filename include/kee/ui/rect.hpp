#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {

/* TODO: change all ui color params to kee::color */

class rect_outline
{
public:
    enum class type;

    rect_outline(rect_outline::type rect_outline_type, float val, const std::optional<raylib::Color>& opt_color);

    rect_outline::type rect_outline_type;
    float val;

    std::optional<raylib::Color> opt_color; /* TODO: change to color i think */
};

enum class rect_outline::type
{
    abs,
    rel_w,
    rel_h
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

class rect : public kee::ui::base
{
public:
    rect(
        const kee::ui::required& reqs, 
        const std::optional<raylib::Color>& color, 
        const kee::pos& x,
        const kee::pos& y,
        const std::variant<kee::dims, kee::border>& dims,
        bool centered, 
        std::optional<kee::ui::rect_outline> border,
        std::optional<kee::ui::rect_roundness> roundness
    );

    raylib::Rectangle get_extended_raw_rect() const;

    std::optional<kee::ui::rect_outline> border; /* TODO: name change to outline */
    std::optional<kee::ui::rect_roundness> roundness;

protected:
    void render_element() const override;

private:
    float get_roundness() const;
};

} // namespace ui
} // namespace kee