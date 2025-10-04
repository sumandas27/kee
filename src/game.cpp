#include "kee/game.hpp"

#include "kee/scene/editor.hpp"

namespace kee {

game::game() :
    curr_scene(std::make_unique<kee::scene::editor>(window, assets))
{ }

void game::main_loop()
{
    while (!window.impl.ShouldClose())
    {
        const bool ctrl_modifier = 
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

        const raylib::Vector2 mouse_pos = raylib::Mouse::GetPosition();
        curr_scene->on_mouse_move(mouse_pos, ctrl_modifier);

        if (raylib::Mouse::IsButtonPressed(MouseButton::MOUSE_BUTTON_LEFT))
            curr_scene->on_mouse_down(mouse_pos, true, ctrl_modifier);
        if (raylib::Mouse::IsButtonReleased(MouseButton::MOUSE_BUTTON_LEFT))
            curr_scene->on_mouse_up(mouse_pos, true, ctrl_modifier);
        if (raylib::Mouse::IsButtonPressed(MouseButton::MOUSE_BUTTON_RIGHT))
            curr_scene->on_mouse_down(mouse_pos, false, ctrl_modifier);
        if (raylib::Mouse::IsButtonReleased(MouseButton::MOUSE_BUTTON_RIGHT))
            curr_scene->on_mouse_up(mouse_pos, false, ctrl_modifier);

        const float mouse_scroll = raylib::Mouse::GetWheelMove();
        if (mouse_scroll != 0.0f)
            curr_scene->on_mouse_scroll(mouse_scroll);

        const float dt = window.impl.GetFrameTime();
        curr_scene->update(dt);

        window.impl.BeginDrawing();
        window.impl.ClearBackground(raylib::Color::Black());
        curr_scene->render();
        window.impl.EndDrawing();
    }
}

} // namespace kee