#pragma once

#include "kee/scene/base.hpp"
#include "kee/scene/manager.hpp"
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

    template <std::derived_from<kee::scene::base> T, typename... Args>
    std::unique_ptr<T> make_scene(Args&&... args);

private:
    NFD::Guard nfd_guard;

    kee::window window;
    kee::global_assets assets;
    raylib::AudioDevice audio;

public:
    kee::scene::manager scene_manager;

    boost::optional<kee::ui::base&> element_keyboard_capture;

private:
    std::unique_ptr<kee::scene::base> curr_scene;
    
    bool main_loop_begun;
    bool game_should_exit;
};

} // namespace kee

#include "kee/game.ipp"