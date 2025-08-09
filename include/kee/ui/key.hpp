#pragma once

#include <array>
#include <deque>

#include "kee/ui/base.hpp"

namespace kee {

/* TODO: next fix */
namespace scene { class beatmap; }

namespace ui {

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
    std::optional<float> hold_next_combo;
};

class key final : public base
{
public:
    class ui_data;

    key(const kee::ui::base& parent, kee::scene::beatmap& beatmap_scene, int key_id, const raylib::Vector2& relative_pos);

    const std::deque<kee::ui::hit_object>& get_hit_objects() const;

    kee::ui::hit_object& front();
    void push(const kee::ui::hit_object& object);
    void pop();

private:
    void update_element(float dt) override;
    void render_element() const override;

    void combo_lose();

    kee::scene::beatmap& beatmap_scene;

    static constexpr float border_width = 0.02f;

    const int keycode;
    const unsigned int id_trans_combo_lost_alpha;

    unsigned int id_rect;
    unsigned int id_combo_lost_rect;

    float combo_lost_time;
    std::deque<kee::ui::hit_object> hit_objects;
};

class key::ui_data
{
public:
    ui_data(int raylib_key, const raylib::Vector2& relative_pos);

    int raylib_key;
    raylib::Vector2 relative_pos;
};

static constexpr std::size_t key_count = 28;
static const std::array<key::ui_data, key_count> key_ui_data = {
    key::ui_data(KeyboardKey::KEY_Q, raylib::Vector2(0.05f, 0.125f)),
    key::ui_data(KeyboardKey::KEY_W, raylib::Vector2(0.15f, 0.125f)),
    key::ui_data(KeyboardKey::KEY_E, raylib::Vector2(0.25f, 0.125f)),
    key::ui_data(KeyboardKey::KEY_R, raylib::Vector2(0.35f, 0.125f)),
    key::ui_data(KeyboardKey::KEY_T, raylib::Vector2(0.45f, 0.125f)),
    key::ui_data(KeyboardKey::KEY_Y, raylib::Vector2(0.55f, 0.125f)),
    key::ui_data(KeyboardKey::KEY_U, raylib::Vector2(0.65f, 0.125f)),
    key::ui_data(KeyboardKey::KEY_I, raylib::Vector2(0.75f, 0.125f)),
    key::ui_data(KeyboardKey::KEY_O, raylib::Vector2(0.85f, 0.125f)),
    key::ui_data(KeyboardKey::KEY_P, raylib::Vector2(0.95f, 0.125f)),
    key::ui_data(KeyboardKey::KEY_A, raylib::Vector2(0.1f, 0.375f)),
    key::ui_data(KeyboardKey::KEY_S, raylib::Vector2(0.2f, 0.375f)),
    key::ui_data(KeyboardKey::KEY_D, raylib::Vector2(0.3f, 0.375f)),
    key::ui_data(KeyboardKey::KEY_F, raylib::Vector2(0.4f, 0.375f)),
    key::ui_data(KeyboardKey::KEY_G, raylib::Vector2(0.5f, 0.375f)),
    key::ui_data(KeyboardKey::KEY_H, raylib::Vector2(0.6f, 0.375f)),
    key::ui_data(KeyboardKey::KEY_J, raylib::Vector2(0.7f, 0.375f)),
    key::ui_data(KeyboardKey::KEY_K, raylib::Vector2(0.8f, 0.375f)),
    key::ui_data(KeyboardKey::KEY_L, raylib::Vector2(0.9f, 0.375f)),
    key::ui_data(KeyboardKey::KEY_Z, raylib::Vector2(0.15f, 0.625f)),
    key::ui_data(KeyboardKey::KEY_X, raylib::Vector2(0.25f, 0.625f)),
    key::ui_data(KeyboardKey::KEY_C, raylib::Vector2(0.35f, 0.625f)),
    key::ui_data(KeyboardKey::KEY_V, raylib::Vector2(0.45f, 0.625f)),
    key::ui_data(KeyboardKey::KEY_B, raylib::Vector2(0.55f, 0.625f)),
    key::ui_data(KeyboardKey::KEY_N, raylib::Vector2(0.65f, 0.625f)),
    key::ui_data(KeyboardKey::KEY_M, raylib::Vector2(0.75f, 0.625f)),
    key::ui_data(KeyboardKey::KEY_COMMA, raylib::Vector2(0.85f, 0.625f)),
    key::ui_data(KeyboardKey::KEY_SPACE, raylib::Vector2(0.5f, 0.875f))
};

} // namespace ui
} // namespace kee