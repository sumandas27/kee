#include "kee/game.hpp"

#include "kee/ui/scene/beatmap.hpp"

namespace kee {

game::game() :
    font_sdf_shader(nullptr, "assets/shaders/sdf.fs"),
    curr_scene(std::make_unique<kee::ui::scene::beatmap>(window))
{ }

void game::main_loop()
{
    while (!window.impl.ShouldClose())
    {
        const float dt = window.impl.GetFrameTime();
        curr_scene->update(dt);

        window.impl.BeginDrawing();
        window.impl.ClearBackground(raylib::Color::Black());

        font_sdf_shader.BeginMode();
        curr_scene->render();
        font_sdf_shader.EndMode();

        window.impl.EndDrawing();
    }
}

} // namespace kee