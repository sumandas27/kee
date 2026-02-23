#pragma once

#include <stack>

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

/**
 * NOTE: Raylib doesn't have supported for nested scissor modes, we implement 
 * it ourselves here. `raylib-cpp` doesn't have Begin/EndScissorMode bindings,
 * so we use them directly.
 */
class nested_scissor_mode
{
public:
    void push(const raylib::Rectangle& scissor_rect);
    void pop();

//private:
    std::stack<raylib::Rectangle> scissor_stack;
};

class game
{
public:
    game();

    void begin_main_loop(); 
    void queue_game_exit();

    template <std::derived_from<kee::scene::base> T, typename... Args>
    std::unique_ptr<kee::scene::base> make_scene(Args&&... args);

private:
    NFD::Guard nfd_guard;

    kee::window window;
    kee::global_assets assets;
    raylib::AudioDevice audio;

    std::unique_ptr<kee::scene::base> curr_scene;

public:
    kee::scene::manager scene_manager;

    kee::nested_scissor_mode scissor_mode;

private:    
    bool main_loop_begun;
    bool game_should_exit;
};

} // namespace kee

#include "kee/game.ipp"