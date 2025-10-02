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

        kee::ui::base& keyboard_handler = element_keyboard_focus.value_or(*curr_scene);
        for (int key = KeyboardKey::KEY_NULL; key <= KeyboardKey::KEY_KB_MENU; key++)
        {
            if (raylib::Keyboard::IsKeyPressed(key))
                keyboard_handler.on_key_down(keyboard_event(key, ctrl_modifier));

            if (raylib::Keyboard::IsKeyReleased(key))
                keyboard_handler.on_key_up(keyboard_event(key, ctrl_modifier));
        }

        const float dt = window.impl.GetFrameTime();
        curr_scene->update(dt);

        window.impl.BeginDrawing();
        window.impl.ClearBackground(raylib::Color::Black());
        curr_scene->render();
        window.impl.EndDrawing();
    }
}

} // namespace kee