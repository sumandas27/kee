#pragma once

#include <deque>

#include "kee/scene/base.hpp"

namespace kee {
namespace scene {

class beatmap final : public kee::scene::base
{
public:
    beatmap(const kee::scene::window& window, kee::global_assets& assets);

    float get_beat() const;

    void combo_increment_with_sound();
    void combo_increment_no_sound();
    void combo_lose();

    const float input_tolerance;
    const float approach_beats;

private:
    void handle_element_events() override;
    void update_element(float dt) override;

    const float load_time;
    const float max_combo_time;

    const float music_start_offset;
    const float music_bpm;

    const unsigned int id_trans_combo_gain;

    unsigned int id_load_rect;
    unsigned int id_progress_rect;
    unsigned int id_combo_text;
    unsigned int id_combo_text_bg;
    unsigned int id_window_border;
    unsigned int id_key_frame;

    raylib::Music music;
    raylib::Sound hitsound;
    raylib::Sound combo_lost_sfx;

    unsigned int combo;
    float combo_time;
    float end_beat;

    float game_time;
};

class beatmap_hit_object
{
public:
    beatmap_hit_object(float beat);
    beatmap_hit_object(float beat, float duration);

    const float beat;
    /**
     * beatmap_hit_objects with duration zero are taps, otherwise they are holds
     */
    const float duration;

    bool hold_is_held;
    bool hold_press_complete;
    std::optional<float> hold_next_combo;
};

class beatmap_key final : public kee::ui::base
{
public:
    beatmap_key(const kee::ui::base::required& reqs, kee::scene::beatmap& beatmap_scene, int key_id, const raylib::Vector2& relative_pos);

    const std::deque<beatmap_hit_object>& get_hit_objects() const;

    beatmap_hit_object& front();
    void push(const beatmap_hit_object& object);
    void pop();

private:
    void handle_element_events() override;
    void render_element_ahead_children() const override;

    void combo_lose();

    kee::scene::beatmap& beatmap_scene;

    const int keycode;
    const unsigned int id_trans_combo_lost_alpha;

    unsigned int id_rect;
    unsigned int id_combo_lost_rect;

    float combo_lost_time;
    std::deque<beatmap_hit_object> hit_objects;
};

} // namespace scene
} // namespace kee