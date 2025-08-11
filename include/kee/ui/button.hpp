#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {

class button final : public kee::ui::base
{
public:
    button(
        const kee::ui::base& parent, 
        kee::pos x, 
        kee::pos y, 
        const std::variant<kee::dims, kee::border>& dimensions, 
        bool centered,
        std::optional<int> z_order,
        bool children_z_order_enabled
    );

    std::function<void()> on_hot;
    std::function<void()> on_down;
    std::function<void()> on_leave;
    std::function<void()> on_click;

    std::function<void(float)> on_update;

private:
    enum class state;

    void update_element(float dt) override;

    button::state button_state;
};

enum class button::state
{
    off,
    hot,
    down
};

} // namespace ui
} // namespace kee