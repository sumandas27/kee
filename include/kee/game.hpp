#pragma once

#include "kee/ui/scene/base.hpp"

namespace kee {

class game
{
public:
    game();

    void main_loop();

private:
    kee::ui::scene::window window;
    raylib::AudioDevice audio;
    raylib::Shader font_sdf_shader;

    std::unique_ptr<kee::ui::scene::base> curr_scene;
};

} // namespace kee