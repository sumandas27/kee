#include "kee/game.hpp"

#include "kee/scene/beatmap.hpp"

namespace kee {

game::game() :
    ctrl_modifier(false),
    curr_scene(std::make_unique<kee::scene::beatmap>(window, assets))
{ }

void game::main_loop()
{
    while (!window.impl.ShouldClose())
    {
        ctrl_modifier = 
            raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_LEFT_CONTROL) ||
            raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_RIGHT_CONTROL);

        kee::ui::base& keyboard_handler = element_keyboard_capture.value_or(*curr_scene);
        for (int key = KeyboardKey::KEY_NULL; key <= KeyboardKey::KEY_KB_MENU; key++)
        {
            if (raylib::Keyboard::IsKeyPressed(key))
                keyboard_handler.on_key_down(key, ctrl_modifier);

            if (raylib::Keyboard::IsKeyReleased(key))
                keyboard_handler.on_key_up(key, ctrl_modifier);
        }

        /* TODO: impl char callback */

        const raylib::Vector2 new_mouse_pos = raylib::Mouse::GetPosition();
        if (mouse_pos != new_mouse_pos)
        {
            curr_scene->on_mouse_move(new_mouse_pos);
            mouse_pos = new_mouse_pos;
        }

        if (raylib::Mouse::IsButtonPressed(MouseButton::MOUSE_BUTTON_LEFT))
            curr_scene->on_mouse_down(mouse_pos, true);
        if (raylib::Mouse::IsButtonReleased(MouseButton::MOUSE_BUTTON_LEFT))
            curr_scene->on_mouse_up(mouse_pos, true);
        if (raylib::Mouse::IsButtonPressed(MouseButton::MOUSE_BUTTON_RIGHT))
            curr_scene->on_mouse_down(mouse_pos, false);
        if (raylib::Mouse::IsButtonReleased(MouseButton::MOUSE_BUTTON_RIGHT))
            curr_scene->on_mouse_up(mouse_pos, false);

        const float dt = window.impl.GetFrameTime();
        curr_scene->update(dt);

        window.impl.BeginDrawing();
        window.impl.ClearBackground(raylib::Color::Black());
        curr_scene->render();
        window.impl.EndDrawing();
    }
}

} // namespace kee