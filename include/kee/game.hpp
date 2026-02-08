#pragma once

#include "kee/scene/base.hpp"
#include "kee/scene/transition.hpp"
#include "kee/global_assets.hpp"

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

    void begin_main_loop(); 
    void queue_game_exit();

    kee::scene::transition scene_manager;

    boost::optional<kee::ui::base&> element_keyboard_capture;

private:
    NFD::Guard nfd_guard;

    kee::window window;
    kee::global_assets assets;
    raylib::AudioDevice audio;

    boost::optional<kee::ui::base&> render_priority;
    std::unique_ptr<kee::scene::base> curr_scene;
    
    bool main_loop_begun;
    bool game_should_exit;
};

} // namespace kee