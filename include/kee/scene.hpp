#pragma once

#include <unordered_map>

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

class key
{
public:
    key(float prop_pos_x, float prop_pos_y);

    const raylib::Vector2 proportional_pos;
    bool is_pressed;
};

class scene
{
public:
    scene(const raylib::Vector2& window_dim);

    void update(float dt);
    void render() const;

private:
    const raylib::Vector2 window_dim;
    const raylib::Vector2 rect_key_grid_dim;
    const float percent_key_space_empty;
    const float start_load_time;

    const float beats_per_tick;

    raylib::Music music;
    raylib::Sound tick;

    float music_start_offset;
    float music_bpm;

    float next_tick_beat;

    int keys_font_size;
    raylib::Font keys_font;
    raylib::Rectangle rect_keys;
    std::unordered_map<int, kee::key> keys;

    float load_time;
    float game_time;
};

} // namespace kee