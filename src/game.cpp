#include "kee/game.hpp"

namespace kee {

window::window()
{
    static constexpr int window_fps = 60;
    //impl.SetConfigFlags(
    //    ConfigFlags::FLAG_WINDOW_TOPMOST |      /* Keep window correctly positioned while fullscreened */
    //    ConfigFlags::FLAG_WINDOW_UNDECORATED |  /* Do not render anything else besides the window screen itself */
    //);
    //impl.Init(raylib::Window::GetWidth(), raylib::Window::GetHeight());

    impl.SetConfigFlags(ConfigFlags::FLAG_WINDOW_HIGHDPI);
    impl.Init(1200, 675);
    impl.SetTargetFPS(window_fps);
}

game::game() :
    font_sdf_shader(nullptr, "assets/shaders/sdf.fs"),
    scene(window.impl.GetSize())
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