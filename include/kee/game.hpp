#pragma once

#include "kee/scene.hpp"

namespace kee {

class game
{
public:
    game();

    void main_loop();

private:
    static constexpr int window_fps = 60;
    raylib::Window window;

    const kee::scene scene;
};

} // namespace kee