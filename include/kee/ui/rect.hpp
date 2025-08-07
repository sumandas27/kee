#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {

class rect final : public kee::ui::base
{
public:
    rect(
        kee::ui::base& parent, 
        const std::optional<raylib::Color>& color, 
        kee::ui::pos x, 
        kee::ui::pos y, 
        const std::variant<kee::ui::dims, kee::ui::border>& dims, 
        std::optional<kee::ui::border> border, 
        bool centered,
        std::optional<int> z_order,
        bool children_z_order_enabled
    );

private:
    void render_element() const override;

    std::optional<kee::ui::border> border;
};

} // namespace ui
} // namespace kee