#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {

/* TODO: update */

class button : public kee::ui::base
{
public:
    enum class event;

    button(
        const kee::ui::base::required& reqs, 
        kee::pos x, 
        kee::pos y, 
        const std::variant<kee::dims, kee::border>& dimensions, 
        bool centered
    );

    std::function<void(button::event)> on_event;
    std::function<void()> on_click_l;
    std::function<void()> on_click_r;

protected:
    //void handle_element_events() override;

private:
    kee::mouse_state button_state;

    std::optional<bool> is_down_l;
};

enum class button::event
{
    on_hot,
    on_down_l,
    on_down_r,
    on_leave
};

} // namespace ui
} // namespace kee