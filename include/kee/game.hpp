#pragma once

#include "kee/scene/base.hpp"
#include "kee/global_assets.hpp"

namespace kee {

class game
{
public:
    game();

    void main_loop();

    boost::optional<kee::ui::base&> element_keyboard_capture;

private:
    NFD::Guard nfd_guard;

    kee::scene::window window;
    kee::global_assets assets;
    raylib::AudioDevice audio;

    boost::optional<kee::ui::base&> render_priority;
    std::unique_ptr<kee::scene::base> curr_scene;
};

} // namespace kee