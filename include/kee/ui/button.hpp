#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {

class button final : public kee::ui::base
{
public:
    enum class event;

    button(
        const kee::ui::base::required& reqs, 
        kee::pos x, 
        kee::pos y, 
        const std::variant<kee::dims, kee::border>& dimensions, 
        const kee::ui::common& common
    );

    std::function<void(button::event)> on_event;
    std::function<void()> on_click;

private:
    void update_element(float dt) override;

    kee::ui::mouse_state button_state;
};

enum class button::event
{
    on_hot,
    on_down,
    on_leave
};

} // namespace ui
} // namespace kee