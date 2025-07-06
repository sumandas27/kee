#include "kee/game.hpp"

namespace kee {

window::window()
{
    static constexpr int window_fps = 60;
    impl.SetTargetFPS(window_fps);
    impl.SetConfigFlags(
        FLAG_WINDOW_TOPMOST |       /* Keep window correctly positioned while fullscreened */
        FLAG_WINDOW_UNDECORATED     /* Do not render anything else besides the window screen itself */
    );
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
        window.impl.ClearBackground(BLACK);
        scene.render();
        window.impl.EndDrawing();
    }
}

} // namespace kee