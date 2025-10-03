#include "kee/ui/button.hpp"

namespace kee {
namespace ui {

button::button(
    const kee::ui::base::required& reqs, 
    kee::pos x, 
    kee::pos y, 
    const std::variant<kee::dims, kee::border>& dimensions, 
    bool centered
) :
    kee::ui::base(reqs, x, y, dimensions, centered),
    on_event([]([[maybe_unused]] button::event button_event){}),
    on_click_l([](){}),
    on_click_r([](){}),
    button_state(mouse_state::off)
{ }

void button::on_element_mouse_move(const raylib::Vector2& mouse_pos)
{
    const raylib::Rectangle raw_rect = get_raw_rect();
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
}

bool button::on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l)
{
    if (button_state != mouse_state::hot)
        return false;

    on_event(is_mouse_l ? button::event::on_down_l : button::event::on_down_r);
    button_state = mouse_state::down;
    is_down_l = is_mouse_l;
    return true;
}

bool button::on_element_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l)
{
    if (button_state != mouse_state::down || is_mouse_l != is_down_l.value())
        return false;

    is_mouse_l ? on_click_l() : on_click_r();
    is_down_l.reset();

    on_event(button::event::on_hot);
    button_state = mouse_state::hot;
    return true;
}

} // namespace ui
} // namespace kee