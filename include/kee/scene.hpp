#pragma once

#include <deque>
#include <print>
#include <unordered_map>
#include <variant>

/**
 * Disabling a bunch of warnings on raylib's source code.
 */
#ifdef _MSC_VER
    #pragma warning(disable: 4458)
#endif
#include "raylib-cpp.hpp"
#ifdef _MSC_VER
    #pragma warning(default: 4458)
#endif

namespace kee {

/* TODO: store combo logic */
/* TODO: top progress bar */
/* TODO: make game */

class hit_object
{
public:
    hit_object(float beat);
    hit_object(float beat, float duration);

    const float beat;
    /**
     * hit_objects with duration zero are taps, otherwise they are holds
     */
    const float duration;

    bool hold_is_held;
    bool hold_press_complete;
};

class key
{
public:
    key(float prop_pos_x, float prop_pos_y);

    const std::deque<kee::hit_object>& get_hit_objects() const;

    void push(const kee::hit_object& object);
    void pop();
    kee::hit_object& front();

    const raylib::Vector2 proportional_pos;
    bool is_pressed;

private:
    std::deque<kee::hit_object> hit_objects;
};

class scene
{
public:
    scene(const raylib::Vector2& window_dim);

    void update(float dt);
    void render() const;

private:
    float get_beat() const;

    const raylib::Vector2 window_dim;
    const raylib::Vector2 rect_key_grid_dim;
    const float percent_key_space_empty;
    const float load_time;

    const float approach_beats;
    const float input_tolerance;

    const float beats_per_tick;

    raylib::Music music;
    /* TODO: rename this to hitsound */
    raylib::Sound tick;

    float music_start_offset;
    float music_bpm;

    int keys_font_size;
    raylib::Font keys_font;
    raylib::Rectangle rect_keys;
    std::unordered_map<int, kee::key> keys;

    float game_time;
};

} // namespace kee