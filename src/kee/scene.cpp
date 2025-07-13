#include "kee/scene.hpp"
#include <iostream>
namespace kee {

tap::tap(float beat) :
    beat(beat)
{ }

float tap::get_last_beat() const
{
    return beat;
}

hold::hold(float beat, float duration) :
    beat(beat),
    duration(duration)
{ }

float hold::get_last_beat() const
{
    return beat + duration;
}

key::key(float prop_pos_x, float prop_pos_y) :
    proportional_pos(prop_pos_x, prop_pos_y),
    is_pressed(false)
{ }

void key::push_hit_object(const kee::hit_object& object)
{
    std::visit([this](auto&& obj)
    {
        if (hit_objects.empty())
            return;

        std::visit([this, &obj](auto&& back)
        {
            if (obj.beat <= back.get_last_beat())
                throw std::invalid_argument("A new hit object must be strictly after all other ones in its key!");
        }, 
        hit_objects.back());
    },
    object);

    hit_objects.push_back(object);
}

scene::scene(const raylib::Vector2& window_dim) :
    window_dim(window_dim),
    rect_key_grid_dim(10, 4),
    percent_key_space_empty(0.05f),
    start_load_time(1.0f),
    beats_per_tick(1.0f),
    music("assets/daft-punk-something-about-us/daft-punk-something-about-us.mp3"),
    tick("assets/sfx/tick.mp3"),
    music_start_offset(1.0f),
    music_bpm(100.0f),
    load_time(start_load_time),
    game_time(0.0f)
{
    const float start_beat = -music_start_offset * music_bpm / 60.0f;
    next_tick_beat = std::min(std::ceil(start_beat / beats_per_tick) * beats_per_tick, 0.0f);

    music.SetLooping(false);
    music.SetVolume(0.1f);
    tick.SetVolume(0.1f);

    const float size_diff = window_dim.x * 0.2f;
    const raylib::Vector2 rect_keys_raw_size(window_dim.x - size_diff, window_dim.y - size_diff);

    if (rect_key_grid_dim.x * rect_keys_raw_size.y >= rect_key_grid_dim.y * rect_keys_raw_size.x)
    {
        rect_keys.width = rect_keys_raw_size.x;
        rect_keys.height = rect_keys.width * rect_key_grid_dim.y / rect_key_grid_dim.x;
    }
    else
    {
        rect_keys.height = rect_keys_raw_size.y;
        rect_keys.width = rect_keys.height * rect_key_grid_dim.x / rect_key_grid_dim.y;
    }

    rect_keys.x = (window_dim.x - rect_keys.width) / 2;
    rect_keys.y = (window_dim.y - rect_keys.height) / 2;

    keys_font_size = static_cast<int>((rect_keys.width / rect_key_grid_dim.x) * (1.0f - percent_key_space_empty) * 0.5f);
    keys_font = raylib::Font("assets/Montserrat-Light.ttf", keys_font_size);

    keys.emplace(KeyboardKey::KEY_Q, kee::key(0.05f, 0.125f));
    keys.emplace(KeyboardKey::KEY_W, kee::key(0.15f, 0.125f));
    keys.emplace(KeyboardKey::KEY_E, kee::key(0.25f, 0.125f));
    keys.emplace(KeyboardKey::KEY_R, kee::key(0.35f, 0.125f));
    keys.emplace(KeyboardKey::KEY_T, kee::key(0.45f, 0.125f));
    keys.emplace(KeyboardKey::KEY_Y, kee::key(0.55f, 0.125f));
    keys.emplace(KeyboardKey::KEY_U, kee::key(0.65f, 0.125f));
    keys.emplace(KeyboardKey::KEY_I, kee::key(0.75f, 0.125f));
    keys.emplace(KeyboardKey::KEY_O, kee::key(0.85f, 0.125f));
    keys.emplace(KeyboardKey::KEY_P, kee::key(0.95f, 0.125f));
    keys.emplace(KeyboardKey::KEY_A, kee::key(0.1f, 0.375f));
    keys.emplace(KeyboardKey::KEY_S, kee::key(0.2f, 0.375f));
    keys.emplace(KeyboardKey::KEY_D, kee::key(0.3f, 0.375f));
    keys.emplace(KeyboardKey::KEY_F, kee::key(0.4f, 0.375f));
    keys.emplace(KeyboardKey::KEY_G, kee::key(0.5f, 0.375f));
    keys.emplace(KeyboardKey::KEY_H, kee::key(0.6f, 0.375f));
    keys.emplace(KeyboardKey::KEY_J, kee::key(0.7f, 0.375f));
    keys.emplace(KeyboardKey::KEY_K, kee::key(0.8f, 0.375f));
    keys.emplace(KeyboardKey::KEY_L, kee::key(0.9f, 0.375f));
    keys.emplace(KeyboardKey::KEY_Z, kee::key(0.15f, 0.625f));
    keys.emplace(KeyboardKey::KEY_X, kee::key(0.25f, 0.625f));
    keys.emplace(KeyboardKey::KEY_C, kee::key(0.35f, 0.625f));
    keys.emplace(KeyboardKey::KEY_V, kee::key(0.45f, 0.625f));
    keys.emplace(KeyboardKey::KEY_B, kee::key(0.55f, 0.625f));
    keys.emplace(KeyboardKey::KEY_N, kee::key(0.65f, 0.625f));
    keys.emplace(KeyboardKey::KEY_M, kee::key(0.75f, 0.625f));
    keys.emplace(KeyboardKey::KEY_COMMA, kee::key(0.85f, 0.625f));
    keys.emplace(KeyboardKey::KEY_SPACE, kee::key(0.5f, 0.875f));

    /* TODO: replace with some file parser eventually */

    keys.at(KeyboardKey::KEY_Q).push_hit_object(kee::tap(0.0f));
    keys.at(KeyboardKey::KEY_W).push_hit_object(kee::tap(4.0f));
    keys.at(KeyboardKey::KEY_P).push_hit_object(kee::tap(8.0f));
    keys.at(KeyboardKey::KEY_O).push_hit_object(kee::tap(12.0f));
}

void scene::update(float dt)
{
    if (load_time > 0.0f)
    {
        load_time -= dt;
        if (load_time <= 0.0f)
            music.Play();
    }
    else
    {
        game_time += dt;

        const float beat = (game_time - music_start_offset) * music_bpm / 60.0f;
        if (beat > next_tick_beat)
        {
            tick.Play();
            next_tick_beat += beats_per_tick;
        }

        music.Update();
    }

    for (auto& [val, key] : keys)
        key.is_pressed = raylib::Keyboard::IsKeyDown(val);
}

void scene::render() const
{
    if (load_time > 0.0f)
    {
        const float load_rect_h = window_dim.y * load_time / start_load_time;
        const raylib::Rectangle load_rect(0, window_dim.y - load_rect_h, window_dim.x, load_rect_h);
        const raylib::Color load_rect_color(255, 255, 255, 10);
        load_rect.Draw(load_rect_color);
    }

    for (const auto& [val, key] : keys)
    {
        static const raylib::Color key_color_regular = raylib::Color::White();
        static const raylib::Color key_color_down = raylib::Color::Green();
        const raylib::Color key_color = key.is_pressed ? key_color_down : key_color_regular;

        const float key_grid_size = rect_keys.width / rect_key_grid_dim.x;
        const float space_empty = percent_key_space_empty * key_grid_size;

        const float w_multiplier = val == KeyboardKey::KEY_SPACE ? 7.0f : 1.0f;
        const float key_w = key_grid_size * w_multiplier - space_empty;
        const float key_h = key_grid_size - space_empty;
        const float key_x = rect_keys.x + rect_keys.width * key.proportional_pos.x - key_w / 2;
        const float key_y = rect_keys.y + rect_keys.height * key.proportional_pos.y - key_h / 2;
        const raylib::Rectangle key_rect(key_x, key_y, key_w, key_h);

        static constexpr float percent_key_border = 0.03f;
        const float key_thickness = key_h * percent_key_border;
        key_rect.DrawLines(key_color, key_thickness);

        const std::string key_text_str = val != KeyboardKey::KEY_SPACE ? std::string(1, static_cast<char>(val)) : "___";
        const raylib::Vector2 key_text_size = keys_font.MeasureText(key_text_str.c_str(), static_cast<float>(keys_font_size), 0.0f);
        const raylib::Vector2 key_text_pos(
            rect_keys.x + rect_keys.width * key.proportional_pos.x - key_text_size.x / 2,
            rect_keys.y + rect_keys.height * key.proportional_pos.y - key_text_size.y / 2
        );

        keys_font.DrawText(key_text_str.c_str(), key_text_pos, static_cast<float>(keys_font_size), 0.0f, key_color);
    }
}

} // namespace kee