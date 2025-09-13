#pragma once

#include "kee/scene/base.hpp"
#include "kee/ui/button.hpp"
#include "kee/ui/image.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/slider.hpp"
#include "kee/ui/text.hpp"
#include "kee/ui/triangle.hpp"

/* BUG: occasionally beats jump to end of song. */
/* BUG: occasionally beat indicators at bottom do NOT update */

namespace kee {
namespace scene {

class object_editor;
class editor_key;

class editor final : public kee::scene::base
{
public:
    static constexpr float beat_lock_threshold = 0.001f;

    static const std::vector<int> prio_to_key;
    static const std::unordered_map<int, int> key_to_prio;

    editor(const kee::scene::window& window, kee::global_assets& assets);

    bool is_music_playing() const;

    float get_beat() const;
    void set_beat(float new_beat);

    void unselect();
    void select(int id);

    const float approach_beats;
    const float beat_step;

private:
    void handle_element_events() override;
    void update_element(float dt) override;

    kee::ui::image_texture play_png;
    kee::ui::image_texture pause_png;

    kee::transition<kee::color>& pause_play_color;
    kee::transition<float>& pause_play_scale;

    /* TODO: how to handle event */
    /* TODO: render indicators based on key selection */
    /* TODO: code indicator selection */
    /* TODO: store unordered_map of (key+beat+duration struct w/ boost::hash_combine) -> (hit object indicators) */
    object_editor& obj_editor;
    kee::ui::slider& music_slider;
    kee::ui::button& pause_play;
    kee::ui::image& pause_play_img;
    kee::ui::text& music_time_text;
    kee::ui::base& key_border;
    kee::ui::base& key_frame;

    const float music_start_offset;
    const float music_bpm;

    std::unordered_map<int, std::reference_wrapper<editor_key>> keys;

    float mouse_wheel_move;

    raylib::Music music;
    float music_time;

    std::vector<int> selected_key_ids;
};

class editor_hit_object
{
public:
    editor_hit_object(float beat);
    editor_hit_object(float beat, float duration);

    float beat;
    float duration;

    bool is_selected;
};

class hit_obj_ui final : public kee::ui::rect
{
public:
    hit_obj_ui(const kee::ui::base::required& reqs, float beat, float duration, float curr_beat, float beat_width, float rel_y);

    void update(float beat, float duration, float curr_beat, float beat_width);

    kee::ui::rect& circle_l;
    kee::ui::rect& circle_r;

private:
    static constexpr float rel_h = 0.1f;
};

class hit_obj_render
{
public:
    hit_obj_render(hit_obj_ui&& render_ui, editor_hit_object& hit_obj_ref);

    hit_obj_ui render_ui;
    std::reference_wrapper<editor_hit_object> hit_obj_ref;
};

/* TODO: add blocks to left and right of indicator */
/* TODO: ability to drag ends of hold objects */

class object_editor final : public kee::ui::rect
{
public:
    static constexpr float beat_width = 4.0f;
    static constexpr float beat_drag_speed = 5.0f;

    object_editor(
        const kee::ui::base::required& reqs,
        const std::unordered_map<int, std::reference_wrapper<editor_key>>& keys,
        const std::vector<int>& selected_key_ids, 
        kee::scene::editor& editor_scene
    );

    kee::ui::base& obj_renderer;

    std::vector<hit_obj_render> obj_render_info;

private:
    void handle_element_events() override;
    void update_element(float dt) override;
    void render_element_behind_children() const override;
    void render_element_ahead_children() const override;

    const std::unordered_map<int, std::reference_wrapper<editor_key>>& keys;
    const std::vector<int>& selected_key_ids;

    kee::scene::editor& editor_scene;

    kee::ui::base& beat_hover_l;
    kee::ui::base& beat_hover_r;
    kee::ui::rect& rect_l;
    kee::ui::rect& rect_r;
    kee::ui::triangle& beat_indicator;

    std::optional<float> beat_drag_start;
    float beat_drag_multiplier;
    float mouse_beat;

    raylib::Vector2 mouse_pos_start;
    bool selected_has_moved;
    bool selected_is_active;

    float selected_reference_beat;
};

class editor_key : public kee::ui::button
{
public:
    editor_key(const kee::ui::base::required& reqs, kee::scene::editor& editor_scene, int key_id);

    std::vector<editor_hit_object> hit_objects;

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
};

} // namespace scene
} // namespace kee