#include "kee/game.hpp"

#include "kee/scene/editor.hpp"

namespace kee {

game::game() :
    curr_scene(std::make_unique<kee::scene::editor>(window, *this, assets))
{ }

void game::main_loop()
{
    while (!window.impl.ShouldClose())
    {
        magic_enum::containers::bitset<kee::mods> mods;
        if (raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_LEFT_CONTROL) || raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_RIGHT_CONTROL))
            mods.set(kee::mods::ctrl, true);
        if (raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_LEFT_SHIFT) || raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_RIGHT_SHIFT))
            mods.set(kee::mods::shift, true);

        kee::ui::base& keyboard_handler = element_keyboard_capture.value_or(*curr_scene);
        for (int key = KeyboardKey::KEY_NULL; key <= KeyboardKey::KEY_KB_MENU; key++)
        {
            if (raylib::Keyboard::IsKeyPressed(key))
                keyboard_handler.on_key_down(key, mods);

            if (raylib::Keyboard::IsKeyReleased(key))
                keyboard_handler.on_key_up(key, mods);
        }

        /* TODO: impl char callback */

        const raylib::Vector2 mouse_pos = raylib::Mouse::GetPosition();
        curr_scene->on_mouse_move(mouse_pos, mods);

        if (raylib::Mouse::IsButtonPressed(MouseButton::MOUSE_BUTTON_LEFT))
            curr_scene->on_mouse_down(mouse_pos, true, mods);
        if (raylib::Mouse::IsButtonReleased(MouseButton::MOUSE_BUTTON_LEFT))
            curr_scene->on_mouse_up(mouse_pos, true, mods);
        if (raylib::Mouse::IsButtonPressed(MouseButton::MOUSE_BUTTON_RIGHT))
            curr_scene->on_mouse_down(mouse_pos, false, mods);
        if (raylib::Mouse::IsButtonReleased(MouseButton::MOUSE_BUTTON_RIGHT))
            curr_scene->on_mouse_up(mouse_pos, false, mods);

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