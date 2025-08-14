#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {

class button final : public kee::ui::base
{
public:
    button(
        const kee::ui::base::required& reqs, 
        kee::pos x, 
        kee::pos y, 
        const std::variant<kee::dims, kee::border>& dimensions, 
        const kee::ui::common& common
    );

    std::function<void()> on_hot;
    std::function<void()> on_down;
    std::function<void()> on_leave;
    std::function<void()> on_click;

    std::function<void(float)> on_update;

private:
    void update_element(float dt) override;

    kee::ui::mouse_state button_state;
};

} // namespace ui
} // namespace kee