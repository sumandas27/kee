#pragma once

#include "kee/scene/base.hpp"

namespace kee {

class game
{
public:
    game();
    ~game();

    void main_loop();

private:
    kee::scene::window window;
    raylib::AudioDevice audio;
    raylib::Shader font_sdf_shader;

    std::unique_ptr<kee::scene::base> curr_scene;
};

} // namespace kee