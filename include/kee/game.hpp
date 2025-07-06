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
    kee::scene scene;
};

} // namespace kee