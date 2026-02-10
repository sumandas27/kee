#include "kee/game.hpp"

#include <avcpp/av.h>

#include "kee/scene/menu.hpp"

namespace kee {

window::window()
{
    av::init();

#ifdef __APPLE__
    static constexpr int window_fps = 60;
#else
    static constexpr int window_fps = 144;
#endif

    impl.SetConfigFlags(
        ConfigFlags::FLAG_BORDERLESS_WINDOWED_MODE |    /* Make window take up the entire screen */
        ConfigFlags::FLAG_WINDOW_UNDECORATED |          /* Remove toolbars when displaying the game */
        ConfigFlags::FLAG_VSYNC_HINT |                  /* Prevent screen tearing */
        ConfigFlags::FLAG_WINDOW_HIGHDPI                /* Correct rendering on Apple machines */
    );

    /* TODO FAR: custom window size and fix ui so fun :D */
    impl.Init(kee::window_w, kee::window_h);
    impl.SetTargetFPS(window_fps);
}

void nested_scissor_mode::push(const raylib::Rectangle& scissor_rect)
{
    if (!scissor_stack.empty())
    {
        const float x = std::max(scissor_stack.top().x, scissor_rect.x);
        const float y = std::max(scissor_stack.top().y, scissor_rect.y);
        const float w = std::min(scissor_stack.top().x + scissor_stack.top().width, scissor_rect.x + scissor_rect.width) - x;
        const float h = std::min(scissor_stack.top().y + scissor_stack.top().height, scissor_rect.y + scissor_rect.height) - y;

        const raylib::Rectangle intersection = (w > 0.f && h > 0.f)
            ? raylib::Rectangle(x, y, w, h)
            : raylib::Rectangle(0, 0, 0, 0);
            
        scissor_stack.push(intersection);
    }
    else
        scissor_stack.push(scissor_rect);

    BeginScissorMode(
        static_cast<int>(scissor_stack.top().x),
        static_cast<int>(scissor_stack.top().y),
        static_cast<int>(scissor_stack.top().width),
        static_cast<int>(scissor_stack.top().height)
    );
}

void nested_scissor_mode::pop()
{
    scissor_stack.pop();

    if (!scissor_stack.empty())
        BeginScissorMode(
            static_cast<int>(scissor_stack.top().x),
            static_cast<int>(scissor_stack.top().y),
            static_cast<int>(scissor_stack.top().width),
            static_cast<int>(scissor_stack.top().height)
        );
    else
        EndScissorMode();
}

game::game() :
    curr_scene(make_scene<kee::scene::menu>(beatmap_dir_info("local_1"), true)),
    scene_manager(kee::scene::required(*this, assets), curr_scene),
    main_loop_begun(false),
    game_should_exit(false)
{ }

void game::begin_main_loop()
{
    if (main_loop_begun)
        throw std::logic_error("kee::game: main loop already begun");

    main_loop_begun = true;
    while (!game_should_exit)
    {
        magic_enum::containers::bitset<kee::mods> mods;
        if (raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_LEFT_CONTROL) || raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_RIGHT_CONTROL))
            mods.set(kee::mods::ctrl, true);
        if (raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_LEFT_SHIFT) || raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_RIGHT_SHIFT))
            mods.set(kee::mods::shift, true);

        for (int key = KeyboardKey::KEY_NULL; key <= KeyboardKey::KEY_KB_MENU; key++)
        {
            if (raylib::Keyboard::IsKeyPressed(key))
                curr_scene->on_key_down(key, mods);

            if (raylib::Keyboard::IsKeyReleased(key))
                curr_scene->on_key_up(key, mods);
        }

        for (int key = raylib::Keyboard::GetCharPressed(); key > 0; key = raylib::Keyboard::GetCharPressed())
            curr_scene->on_char_press(static_cast<char>(key));

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
        scene_manager.update(dt);

        window.impl.BeginDrawing();
        window.impl.ClearBackground(raylib::Color::Black());

        curr_scene->render();
        scene_manager.render();

        window.impl.EndDrawing();
    }
}

void game::queue_game_exit()
{
    game_should_exit = true;
}

} // namespace kee