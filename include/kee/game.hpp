#pragma once

#include "kee/scene/base.hpp"
#include "kee/global_assets.hpp"

namespace kee {

class game
{
public:
    game();

    void main_loop();

private:
    kee::scene::window window;
    kee::global_assets assets;
    raylib::AudioDevice audio;

    std::unique_ptr<kee::scene::base> curr_scene;
};

} // namespace kee