#pragma once

#include "kee/scene/base.hpp"
#include "kee/ui/button.hpp"
#include "kee/ui/image.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/slider.hpp"
#include "kee/ui/text.hpp"
#include "kee/ui/triangle.hpp"

/* TODO: abstract shared info between `editor` and `beatmap` scene */

namespace kee {
namespace scene {

class editor_key;

class editor final : public kee::scene::base
{
public:
    static constexpr float beat_lock_threshold = 0.001f;

    editor(const kee::scene::window& window, kee::global_assets& assets);

    float get_beat() const;

    void unselect();
    void select(int id);

    const float approach_beats;

private:
    void handle_element_events() override;
    void update_element(float dt) override;
    void render_element_behind_children() const override;

    kee::ui::image_texture play_png;
    kee::ui::image_texture pause_png;

    kee::transition<kee::color>& pause_play_color;
    kee::transition<float>& pause_play_scale;

    /* TODO: render indicators based on key selection */
    /* TODO: code indicator selection */
    /* TODO: store unordered_map of (key+beat+duration struct w/ boost::hash_combine) -> (hit object indicators) */
    kee::ui::base& beat_ticks_frame;
    kee::ui::triangle& beat_indicator;
    kee::ui::slider& music_slider;
    kee::ui::button& pause_play;
    kee::ui::image& pause_play_img;
    kee::ui::text& music_time_text;
    kee::ui::base& key_border;
    kee::ui::base& key_frame;

    const float music_start_offset;
    const float music_bpm;

    const float beat_step;

    std::unordered_map<int, std::reference_wrapper<editor_key>> keys;

    float mouse_wheel_move;

    bool is_music_playing;
    raylib::Music music;

    std::vector<int> selected_key_ids;
};

class editor_hit_object
{
public:
    editor_hit_object(float beat);
    editor_hit_object(float beat, float duration);

    const float beat;
    const float duration;
};

class editor_key : public kee::ui::button
{
public:
    editor_key(const kee::ui::base::required& reqs, kee::scene::editor& editor_scene, int key_id);

    const std::vector<editor_hit_object>& get_hit_objects() const;

    void push(const editor_hit_object& object);

    bool is_selected;

private:
    void handle_element_events() override;
    void render_element_behind_children() const override;
    void render_element_ahead_children() const override;

    void render_hit_objects() const;

    kee::scene::editor& editor_scene;

    kee::ui::rect& frame;
    kee::ui::text& key_text;

    const int key_id;

    bool is_control_clicked;
    std::vector<editor_hit_object> hit_objects;
};

} // namespace scene
} // namespace kee