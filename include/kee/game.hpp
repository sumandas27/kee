#pragma once

#include "kee/scene/base.hpp"
#include "kee/global_assets.hpp"

/**
 * TODO: (1) undo/redo controls
 * TODO: (2) hit obj inspector
 * TODO: (3) move everything down, make space for top menu
 * TODO: (4) write out top menu: METADATA EDITOR DECORATION TIMING X (exit)
 * TODO: (5) transition from editor to for example metadata screen, prob keep all existing element under one base widget
 * TODO: (6) write out metadata screen
 */

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

    boost::optional<kee::ui::base&> render_priority;
    boost::optional<kee::ui::base&> element_keyboard_capture;
    std::unique_ptr<kee::scene::base> curr_scene;
};

} // namespace kee