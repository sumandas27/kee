#include "kee/scene.hpp"

#include <iostream>

namespace kee {

key::key(float prop_pos_x, float prop_pos_y) :
    proportional_pos(prop_pos_x, prop_pos_y),
    is_pressed(false)
{ }

scene::scene(const raylib::Vector2& window_dim) :
    rect_key_grid_dim(10, 4),
    percent_key_space_empty(0.05f),
    percent_key_border(0.03f)
{
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
}

void scene::render() const
{
    for (const auto& [val, key] : keys)
    {
        const float key_grid_size = rect_keys.width / rect_key_grid_dim.x;
        const float space_empty = percent_key_space_empty * key_grid_size;

        const float w_multiplier = val == KeyboardKey::KEY_SPACE ? 7.0f : 1.0f;
        const float key_w = key_grid_size * w_multiplier - space_empty;
        const float key_h = key_grid_size - space_empty;
        const float key_x = rect_keys.x + rect_keys.width * key.proportional_pos.x - key_w / 2;
        const float key_y = rect_keys.y + rect_keys.height * key.proportional_pos.y - key_h / 2;
        const raylib::Rectangle key_rect(key_x, key_y, key_w, key_h);

        const float key_thickness = key_h * percent_key_border;
        DrawRectangleLinesEx(key_rect, key_thickness, WHITE);
    }
}

}