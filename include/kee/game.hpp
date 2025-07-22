#pragma once

#include "kee/scene.hpp"

namespace kee {

class window
{
public:
    window();

    raylib::Window impl;
};


class game
{
public:
    game();

    void main_loop();

private:
    kee::window window;
    raylib::AudioDevice audio;
    raylib::Shader font_sdf_shader;

    kee::scene scene;
};

} // namespace kee