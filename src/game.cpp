#include "kee/game.hpp"

#include "kee/scene/beatmap.hpp"

namespace kee {

game::game() :
    font_sdf_shader(nullptr, "assets/shaders/sdf.fs"),
    curr_scene(std::make_unique<kee::scene::beatmap>(window))
{ 
    font_sdf_shader.BeginMode();
}

game::~game()
{
    font_sdf_shader.EndMode();
}

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