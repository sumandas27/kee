#include "kee/game.hpp"

namespace kee {

window::window()
{
    static constexpr int window_fps = 60;
    impl.SetTargetFPS(window_fps);
    impl.SetConfigFlags(FLAG_WINDOW_TOPMOST | FLAG_WINDOW_UNDECORATED);
    impl.Init(GetScreenWidth(), GetScreenHeight());
}

game::game() :
    scene(window.impl.GetSize())
{ }

void game::main_loop()
{
    while (!window.impl.ShouldClose())
    {
        window.impl.BeginDrawing();
        scene.render();
        window.impl.EndDrawing();
    }
}

} // namespace kee