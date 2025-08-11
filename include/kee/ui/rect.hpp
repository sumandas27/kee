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

class rect final : public kee::ui::base
{
public:
    rect(
        const kee::ui::base& parent, 
        const std::optional<raylib::Color>& color, 
        kee::pos x, 
        kee::pos y, 
        const std::variant<kee::dims, kee::border>& dims, 
        std::optional<const kee::ui::rect_border> border, 
        const kee::ui::common& common
    );

private:
    void render_element() const override;

    const std::optional<const kee::ui::rect_border> border;
};

} // namespace ui
} // namespace kee