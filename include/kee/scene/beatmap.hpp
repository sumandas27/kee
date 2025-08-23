#pragma once

#include <deque>

#include "kee/scene/base.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/text.hpp"

namespace kee {
namespace scene {

class beatmap_key;

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
    void render_element_behind_children() const override;

    kee::transition<float>& combo_gain;

    kee::ui::rect& progress_rect;
    kee::ui::text& combo_text;
    kee::ui::text& combo_text_bg;
    kee::ui::base& window_border;
    kee::ui::base& key_frame;

    const float load_time;
    const float max_combo_time;

    const float music_start_offset;
    const float music_bpm;

    std::unordered_map<int, std::reference_wrapper<beatmap_key>> keys;

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

    kee::transition<float>& combo_lost_alpha;

    kee::ui::rect& frame;
    kee::ui::rect& frame_combo_lost;
    kee::ui::text& key_text;

    const int keycode;

    float combo_lost_time;
    std::deque<beatmap_hit_object> hit_objects;
};

} // namespace scene
} // namespace kee