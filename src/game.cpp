#include "kee/game.hpp"

#include "kee/scene/editor.hpp"

namespace kee {

game::game() :
    curr_scene(std::make_unique<kee::scene::editor>(window))
{ }

void game::main_loop()
{
    while (!window.impl.ShouldClose())
    {
        const float dt = window.impl.GetFrameTime();
        curr_scene->update(dt);

        window.impl.BeginDrawing();
        window.impl.ClearBackground(raylib::Color::Black());
        curr_scene->render();
        window.impl.EndDrawing();
    }
}

} // namespace kee