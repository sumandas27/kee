#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {

class rect_border
{
public:
    enum class type;

    rect_border(rect_border::type rect_border_type, float val);

    const rect_border::type rect_border_type;
    const float val;
};

enum class rect_border::type
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

    rect_roundness(rect_roundness::type rect_roundness_type, float val);

    const rect_roundness::type rect_roundness_type;
    const float val;
};

enum class rect_roundness::type
{
    abs,
    rel_w,
    rel_h
};

class rect final : public kee::ui::base
{
public:
    rect(
        const kee::ui::base& parent, 
        const std::optional<raylib::Color>& color, 
        kee::pos x, 
        kee::pos y, 
        const std::variant<kee::dims, kee::border>& dims, 
        std::optional<kee::ui::rect_border> border,
        std::optional<kee::ui::rect_roundness> roundness, 
        const kee::ui::common& common
    );

private:
    void render_element() const override;

    const std::optional<kee::ui::rect_border> border;
    const std::optional<kee::ui::rect_roundness> roundness;
};

} // namespace ui
} // namespace kee