#include "kee/game.hpp"

namespace kee {

game::game()
{
    SetTargetFPS(game::window_fps);
    SetConfigFlags(FLAG_WINDOW_TOPMOST | FLAG_WINDOW_UNDECORATED);
    window.Init(GetScreenWidth(), GetScreenHeight());
}

void game::main_loop()
{
    while (!window.ShouldClose())
    {
        BeginDrawing();
        scene.render();
        EndDrawing();
    }
}

} // namespace kee