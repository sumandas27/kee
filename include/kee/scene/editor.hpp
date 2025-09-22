#pragma once

#include <map>

#include "kee/scene/base.hpp"
#include "kee/ui/button.hpp"
#include "kee/ui/image.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/slider.hpp"
#include "kee/ui/text.hpp"
#include "kee/ui/triangle.hpp"

/* TODO: add right-hand bar to edit hit objects */

namespace kee {
namespace scene {

class object_editor;
class editor_key;

/**
 * These objects will belong in an `std::set`, whose keys store the `beat` of the object.
 */
class editor_hit_object
{
public:
    editor_hit_object(int key, float duration);

    int key;

    float duration;
    bool is_selected;
};

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

    object_editor& obj_editor;

    const float approach_beats;
    const float beat_step;

    std::unordered_map<int, std::reference_wrapper<editor_key>> keys;

private:
    void handle_element_events() override;
    void update_element(float dt) override;

    kee::ui::image_texture play_png;
    kee::ui::image_texture pause_png;

    kee::transition<kee::color>& pause_play_color;
    kee::transition<float>& pause_play_scale;

    kee::ui::rect& inspector_rect;
    kee::ui::slider& music_slider;
    kee::ui::button& pause_play;
    kee::ui::image& pause_play_img;
    kee::ui::text& music_time_text;
    kee::ui::base& key_border;
    kee::ui::base& key_frame;

    const float music_start_offset;
    const float music_bpm;

    float mouse_wheel_move;

    raylib::Music music;
    float music_time;

    std::vector<int> selected_key_ids;
};

class hit_obj_ui final : public kee::ui::rect
{
public:
    hit_obj_ui(const kee::ui::base::required& reqs, float beat, float duration, float curr_beat, float beat_width, float rel_y);

    void select();
    void unselect();

    void update(float beat, float duration, float curr_beat, float beat_width);

    kee::ui::rect& circle_l;
    kee::ui::rect& circle_r;

private:
    static constexpr float rel_h = 0.1f;
};

class hit_obj_render
{
public:
    hit_obj_render(hit_obj_ui&& render_ui, std::map<float, editor_hit_object>::iterator hit_obj_ref);

    hit_obj_ui render_ui;
    std::map<float, editor_hit_object>::iterator hit_obj_ref;
};

class hit_obj_update_return
{
public:
    hit_obj_update_return(float beat, float duration);

    float beat;
    float duration;
};

class hit_obj_node
{
public:
    hit_obj_node(hit_obj_render& ref, hit_obj_update_return obj_new, std::map<float, editor_hit_object>::node_type node);

    hit_obj_render& ref;
    hit_obj_update_return obj_new;
    std::map<float, editor_hit_object>::node_type node;
};

class new_hit_obj_data
{
public:
    new_hit_obj_data(int key, float rel_y, float click_beat, float current_beat);

    int key;
    float rel_y;

    float click_beat;
    float current_beat;
};

class drag_selection
{
public:
    drag_selection(std::map<float, editor_hit_object>::iterator hold_obj_ref);

    std::map<float, editor_hit_object>::iterator hold_obj_ref;
    bool is_left;
};

class object_editor final : public kee::ui::rect
{
public:
    static constexpr float hit_objs_rel_y = 0.1f;
    static constexpr float hit_objs_rel_h = 0.6f;

    static constexpr float beat_width = 5.0f;
    static constexpr float beat_drag_speed = 7.0f;

    object_editor(
        const kee::ui::base::required& reqs,
        const std::unordered_map<int, std::reference_wrapper<editor_key>>& keys,
        const std::vector<int>& selected_key_ids, 
        kee::scene::editor& editor_scene
    );

    void reset_render_hit_objs();
    void attempt_add_hit_obj();

    kee::ui::base& obj_renderer;
    std::vector<hit_obj_render> obj_render_info;

    std::optional<new_hit_obj_data> new_hit_object;

private:
    void handle_element_events() override;
    void update_element(float dt) override;
    void render_element_behind_children() const override;
    void render_element_ahead_children() const override;

    hit_obj_update_return hit_obj_update_info(const hit_obj_render& hit_obj, float beat_drag_diff) const;

    const std::unordered_map<int, std::reference_wrapper<editor_key>>& keys;
    const std::vector<int>& selected_key_ids;

    kee::scene::editor& editor_scene;

    kee::ui::base& beat_hover_l;
    kee::ui::base& beat_hover_r;
    kee::ui::rect& settings_rect;
    kee::ui::rect& key_label_rect;
    kee::ui::triangle& beat_indicator;

    std::optional<float> beat_drag_start;
    std::optional<kee::ui::rect> selection_rect;
    std::optional<drag_selection> hit_obj_drag_selection;

    float mouse_beat;
    float beat_drag_multiplier;

    raylib::Vector2 mouse_pos_start;
    bool selected_has_moved;
    bool selected_is_active;

    float selected_beat;
    float selected_reference_beat;

    bool new_hit_obj_from_editor;
};

class editor_key : public kee::ui::button
{
public:
    editor_key(const kee::ui::base::required& reqs, kee::scene::editor& editor_scene, int key_id);

    std::map<float, editor_hit_object> hit_objects;

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