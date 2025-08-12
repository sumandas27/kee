#pragma once

#include "kee/scene/base.hpp"

/* TODO: create asset manager */

namespace kee {

class game
{
public:
    game();

    void main_loop();

private:
    kee::scene::window window;
    raylib::AudioDevice audio;

    std::unique_ptr<kee::scene::base> curr_scene;
};

} // namespace kee