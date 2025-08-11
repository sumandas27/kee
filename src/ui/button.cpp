#include "kee/ui/button.hpp"

namespace kee {
namespace ui {

button::button(
    const kee::ui::base& parent, 
    kee::pos x, 
    kee::pos y, 
    const std::variant<kee::dims, kee::border>& dimensions, 
    const kee::ui::common& common
) :
    kee::ui::base(parent, x, y, dimensions, common),
    on_hot([](){}),
    on_down([](){}),
    on_leave([](){}),
    on_click([](){}),
    on_update([]([[maybe_unused]] float dt){}),
    button_state(state::off)
{ }

void button::update_element(float dt)
{
    on_update(dt);

    const raylib::Vector2 mouse_pos = raylib::Mouse::GetPosition();
    const raylib::Rectangle raw_rect = get_raw_rect();
    const bool is_mouse_on_button =
        mouse_pos.x >= raw_rect.x && mouse_pos.x <= raw_rect.x + raw_rect.width &&
        mouse_pos.y >= raw_rect.y && mouse_pos.y <= raw_rect.y + raw_rect.height;

    if (!is_mouse_on_button && button_state != state::off)
    {
        on_leave();
        button_state = state::off;
    }
    else if (is_mouse_on_button && button_state == state::off)
    {
        on_hot();
        button_state = state::hot;
    }
    else if (raylib::Mouse::IsButtonPressed(MouseButton::MOUSE_BUTTON_LEFT) && button_state == state::hot)
    {
        on_down();
        button_state = state::down;
    }
    else if (raylib::Mouse::IsButtonReleased(MouseButton::MOUSE_BUTTON_LEFT) && button_state == state::down)
    {
        on_click();
        on_hot();
        button_state = state::hot;
    }
}

} // namespace ui
} // namespace kee