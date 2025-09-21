#include "kee/ui/button.hpp"

namespace kee {
namespace ui {

button::button(
    const kee::ui::base::required& reqs, 
    kee::pos x, 
    kee::pos y, 
    const std::variant<kee::dims, kee::border>& dimensions, 
    const kee::ui::common& common
) :
    kee::ui::base(reqs, x, y, dimensions, common),
    on_event([]([[maybe_unused]] button::event button_event){}),
    on_click_l([](){}),
    on_click_r([](){}),
    button_state(mouse_state::off)
{ }

void button::handle_element_events()
{
    const raylib::Vector2 mouse_pos = raylib::Mouse::GetPosition();
    const raylib::Rectangle raw_rect = get_raw_rect();
    
    const bool is_mouse_down_l = raylib::Mouse::IsButtonPressed(MouseButton::MOUSE_BUTTON_LEFT); 
    const bool is_mouse_down_r = raylib::Mouse::IsButtonPressed(MouseButton::MOUSE_BUTTON_RIGHT);
    
    const bool is_mouse_on_button =
        mouse_pos.x >= raw_rect.x && mouse_pos.x <= raw_rect.x + raw_rect.width &&
        mouse_pos.y >= raw_rect.y && mouse_pos.y <= raw_rect.y + raw_rect.height;

    if (!is_mouse_on_button && button_state != mouse_state::off)
    {
        on_event(button::event::on_leave);
        button_state = mouse_state::off;
        is_down_l.reset();
    }
    else if (is_mouse_on_button && button_state == mouse_state::off)
    {
        on_event(button::event::on_hot);
        button_state = mouse_state::hot;
    }
    else if ((is_mouse_down_l || is_mouse_down_r) && button_state == mouse_state::hot)
    {
        on_event(is_mouse_down_l ? button::event::on_down_l : button::event::on_down_r);
        button_state = mouse_state::down;
        is_down_l = is_mouse_down_l;
    }
    else if (button_state == mouse_state::down)
    {
        const bool is_mouse_released_l = raylib::Mouse::IsButtonReleased(MouseButton::MOUSE_BUTTON_LEFT) && is_down_l.value();
        const bool is_mouse_released_r = raylib::Mouse::IsButtonReleased(MouseButton::MOUSE_BUTTON_RIGHT) && !is_down_l.value();
        if (!is_mouse_released_l && !is_mouse_released_r)
            return;

        is_mouse_released_l ? on_click_l() : on_click_r();
        is_down_l.reset();

        on_event(button::event::on_hot);
        button_state = mouse_state::hot;
    }
}

} // namespace ui
} // namespace kee