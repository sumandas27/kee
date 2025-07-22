#include "kee/game.hpp"

namespace kee {

window::window()
{
    static constexpr int window_fps = 60;
    impl.SetConfigFlags(
        ConfigFlags::FLAG_WINDOW_TOPMOST |      /* Keep window correctly positioned while fullscreened */
        ConfigFlags::FLAG_WINDOW_UNDECORATED    /* Do not render anything else besides the window screen itself */
    );
    impl.Init(raylib::Window::GetWidth(), raylib::Window::GetHeight());
    impl.SetTargetFPS(window_fps);
}

game::game() :
    scene(window.impl.GetSize()),
    font_sdf_shader(nullptr, "assets/shaders/sdf.fs")
{ }

void game::main_loop()
{
    while (!window.impl.ShouldClose())
    {
        const float dt = window.impl.GetFrameTime();
        scene.update(dt);

        window.impl.BeginDrawing();
        window.impl.ClearBackground(raylib::Color::Black());

        font_sdf_shader.BeginMode();
        scene.render();
        font_sdf_shader.EndMode();

        window.impl.EndDrawing();
    }
}

} // namespace kee