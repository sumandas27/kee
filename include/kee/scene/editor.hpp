#pragma once

#include <deque>

#include "kee/scene/base.hpp"
#include "kee/ui/button.hpp"
#include "kee/ui/dropdown.hpp"
#include "kee/ui/image.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/slider.hpp"
#include "kee/ui/text.hpp"
#include "kee/ui/triangle.hpp"

namespace kee {
namespace scene {

/* TODO: add hitsounds while composing */

class compose_tab;

/**
 * These objects will belong in an `std::set`, whose keys store the `beat` of the object.
 */
class compose_tab_hit_object
{
public:
    compose_tab_hit_object(int key, float duration);

    int key;
    float duration;
};

class hit_obj_ui final : public kee::ui::rect
{
public:
    hit_obj_ui(const kee::ui::base::required& reqs, float beat, float duration, float curr_beat, float beat_width, std::size_t key_idx, std::size_t rendered_key_count);

    void select();
    void unselect();
    bool is_selected() const;

    std::size_t get_key_idx() const;
    void set_key_idx(std::size_t new_key_idx);

    void update_pos_x(float beat, float duration, float curr_beat, float beat_width);

    kee::ui::handle<kee::ui::rect> circle_l;
    kee::ui::handle<kee::ui::rect> circle_r;

    std::size_t start_key_idx;

private:
    static constexpr float rel_h = 0.1f;

    bool selected;

    std::size_t rendered_key_count;
    std::size_t key_idx;
};

class hit_obj_position
{
public:
    hit_obj_position(float beat, float duration);

    bool operator==(const hit_obj_position&) const = default;

    float beat;
    float duration;
};

class hit_obj_metadata
{
public:
    hit_obj_metadata(int key, float beat, float duration);

    int key;
    hit_obj_position position;
};

class new_hit_obj_data
{
public:
    new_hit_obj_data(int key, std::size_t key_idx, float click_beat, float current_beat, bool from_compose_tab);

    int key;
    std::size_t key_idx;

    float click_beat;
    float current_beat;

    bool from_compose_tab;
};

class compose_tab_event
{
public:
    compose_tab_event(const std::vector<hit_obj_metadata>& added, const std::vector<hit_obj_metadata>& removed);

    std::vector<hit_obj_metadata> added;
    std::vector<hit_obj_metadata> removed;
};

class compose_tab_key : public kee::ui::button
{
public:
    compose_tab_key(const kee::ui::base::required& reqs, kee::scene::compose_tab& compose_tab_scene, int key_id);

    std::map<float, compose_tab_hit_object> hit_objects;

    std::vector<kee::ui::rect> hit_obj_rects;
    kee::ui::handle<kee::ui::rect> frame;
    kee::ui::handle<kee::ui::text> key_text;

    bool is_selected;

private:
    void update_element(float dt) override;
    void render_element() const override;

    kee::scene::compose_tab& compose_tab_scene;

    const int key_id;
};

class selection_box_info
{
public:
    selection_box_info(kee::ui::rect&& rect, float rel_y_start);

    kee::ui::rect rect;
    float rel_y_start;
};

class selection_key_drag_info
{
public:
    selection_key_drag_info(std::size_t key_idx_lo, std::size_t key_idx_hi, std::size_t key_idx_start);

    const std::size_t key_idx_lo;
    const std::size_t key_idx_hi;

    std::size_t key_idx_start;
};

enum class drag_selection
{
    none,
    drag_l,
    drag_r
};

class selection_obj_info
{
public:
    selection_obj_info(const std::optional<selection_key_drag_info>& key_idx_info, drag_selection hit_obj_drag_selection);

    std::optional<selection_key_drag_info> key_idx_info;
    drag_selection hit_obj_drag_selection;

    bool selected_has_moved;
};

class selection_info
{
public:
    selection_info(
        float beat_drag_start, 
        raylib::Vector2 mouse_pos_start,
        std::variant<selection_box_info, selection_obj_info>&& variant
    );

    float beat_drag_start;
    raylib::Vector2 mouse_pos_start;

    std::variant<selection_box_info, selection_obj_info> variant;

    bool has_moved;
};

class hit_obj_ui_key
{
public:
    hit_obj_ui_key(std::map<float, compose_tab_hit_object>& map, std::map<float, compose_tab_hit_object>::iterator it);

    hit_obj_metadata get_metadata() const;

    std::map<float, compose_tab_hit_object>::node_type extract();
    void delete_from_map();

    bool operator<(const hit_obj_ui_key& other) const;

private:
    boost::optional<std::map<float, compose_tab_hit_object>&> map;
    std::map<float, compose_tab_hit_object>::iterator it;
};

class hit_obj_node
{
public:
    hit_obj_node(
        std::map<hit_obj_ui_key, hit_obj_ui>::node_type&& node_render_param,
        const hit_obj_metadata& old_obj,
        const hit_obj_metadata& new_obj
    );

    std::map<hit_obj_ui_key, hit_obj_ui>::node_type node_render;
    std::map<float, compose_tab_hit_object>::node_type node_obj;

    hit_obj_metadata old_obj;
    hit_obj_metadata new_obj;
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
        const std::vector<int>& selected_key_ids,
        std::unordered_map<int, kee::ui::handle<compose_tab_key>>& keys,
        kee::scene::compose_tab& compose_tab_scene
    );

    void reset_render_hit_objs();
    bool delete_selected_hit_objs();
    void attempt_add_hit_obj();

    const std::vector<int>& get_keys_to_render() const;

    kee::ui::handle<kee::ui::base> obj_renderer;
    std::map<hit_obj_ui_key, hit_obj_ui> obj_render_info;

    std::optional<new_hit_obj_data> new_hit_object;

private:
    void on_element_mouse_move(const raylib::Vector2& mouse_pos, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;

    void update_element(float dt) override;
    void render_element() const override;

    float get_beat_drag_diff() const;
    hit_obj_position hit_obj_update_info(const std::pair<const hit_obj_ui_key, hit_obj_ui>& map_elem, float beat_drag_diff) const;

    void handle_mouse_up(bool is_mouse_l);
    void attempt_move_op();

    std::size_t get_mouse_key_idx(float mouse_pos_y) const;

    const std::vector<int>& selected_key_ids;

    std::unordered_map<int, kee::ui::handle<compose_tab_key>>& keys;
    kee::scene::compose_tab& compose_tab_scene;

    std::vector<kee::ui::rect> beat_render_rects;
    std::vector<kee::ui::text> whole_beat_texts;
    std::optional<hit_obj_ui> new_hit_obj_render;

    kee::ui::handle<kee::ui::base> beat_hover_l;
    kee::ui::handle<kee::ui::base> beat_hover_r;
    kee::ui::handle<kee::ui::rect> settings_rect;
    kee::ui::handle<kee::ui::rect> key_label_rect;
    kee::ui::handle<kee::ui::triangle> beat_indicator;

    std::vector<kee::ui::handle<kee::ui::text>> key_labels;

    std::optional<selection_info> selection;
    float selected_beat;
    float selected_reference_beat;

    float mouse_beat;
    float beat_drag_multiplier;
};

class compose_tab final : public kee::ui::base
{
public:
    static constexpr float beat_lock_threshold = 0.001f;

    static const std::vector<int> prio_to_key;
    static const std::unordered_map<int, int> key_to_prio;

    compose_tab(const kee::ui::base::required& reqs);

    int get_ticks_per_beat() const;
    bool is_music_playing() const;
    bool is_beat_snap_enabled() const;
    bool is_key_lock_enabled() const;

    float get_beat() const;
    void set_beat(float new_beat);

    void unselect();
    void select(int id);

    void add_event(const compose_tab_event& e);
    void process_event(const compose_tab_event& e);

    const float approach_beats;

    kee::ui::handle<object_editor> obj_editor;

private:
    static constexpr std::size_t tick_freq_count = 8;
    static constexpr std::array<int, tick_freq_count> tick_freqs = { 1, 2, 3, 4, 6, 8, 12, 16 };

    static constexpr std::array<float, 6> playback_speeds = { 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f };

    bool on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_scroll(float mouse_scroll) override;

    void update_element(float dt) override;

    void set_tick_freq_idx(std::size_t new_tick_freq_idx);

    const float music_start_offset;
    const float music_bpm;

    kee::image_texture pause_png;
    kee::image_texture arrow_png;

    std::size_t tick_freq_idx;

    kee::transition<kee::color>& pause_play_color;
    kee::transition<float>& pause_play_scale;
    kee::transition<kee::color>& beat_snap_button_color;
    kee::transition<float>& beat_snap_button_outline;
    kee::transition<kee::color>& key_lock_button_color;
    kee::transition<float>& key_lock_button_outline;
    kee::transition<kee::color>& tick_l_button_color;
    kee::transition<kee::color>& tick_r_button_color;
    kee::transition<float>& tick_l_button_scale;
    kee::transition<float>& tick_r_button_scale;
    kee::transition<float>& tick_curr_rect_x;

    kee::ui::handle<kee::ui::rect> inspector_rect;

    kee::ui::handle<kee::ui::button> beat_snap_button;
    kee::ui::handle<kee::ui::rect> beat_snap_button_rect;
    kee::ui::handle<kee::ui::text> beat_snap_text;

    kee::ui::handle<kee::ui::button> key_lock_button;
    kee::ui::handle<kee::ui::rect> key_lock_button_rect;
    kee::ui::handle<kee::ui::text> key_lock_text;

    kee::ui::handle<kee::ui::text> tick_text;
    kee::ui::handle<kee::ui::button> tick_l_button;
    kee::ui::handle<kee::ui::image> tick_l_img;
    kee::ui::handle<kee::ui::button> tick_r_button;
    kee::ui::handle<kee::ui::image> tick_r_img;

    kee::ui::handle<kee::ui::rect> tick_frame;
    kee::ui::handle<kee::ui::rect> tick_curr_rect;

    std::vector<kee::ui::handle<kee::ui::text>> tick_frame_texts;
    std::vector<kee::ui::handle<kee::ui::button>> tick_frame_buttons;
    std::vector<std::reference_wrapper<kee::transition<kee::color>>> tick_frame_text_colors;

    kee::ui::handle<kee::ui::text> playback_speed_text;
    kee::ui::handle<kee::ui::dropdown> playback_dropdown;

    kee::ui::handle<kee::ui::slider> music_slider;
    kee::ui::handle<kee::ui::button> pause_play;
    kee::ui::handle<kee::ui::image> pause_play_img;
    kee::ui::handle<kee::ui::text> music_time_text;
    kee::ui::handle<kee::ui::base> key_border;
    kee::ui::handle<kee::ui::base> key_frame;

    std::vector<kee::ui::handle<kee::ui::base>> key_holders;
    std::unordered_map<int, kee::ui::handle<compose_tab_key>> keys;

    float mouse_wheel_move;
    bool is_beat_snap;
    bool is_key_locked;

    raylib::Music music;
    float music_time;

    std::vector<hit_obj_metadata> clipboard;
    float clipboard_reference_beat;

    std::size_t event_history_idx;
    std::deque<compose_tab_event> event_history;

    std::vector<int> selected_key_ids;
};

class editor final : public kee::scene::base
{
public:
    editor(const kee::scene::window& window, kee::global_assets& assets);

private:
    enum class tabs;

    bool on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_key_up(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_char_press(char c) override;

    bool on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;
    void on_element_mouse_move(const raylib::Vector2& mouse_pos, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_scroll(float scroll_amount) override;

    void update_element(float dt) override;
    void render_element() const override;

    std::vector<std::reference_wrapper<kee::transition<kee::color>>> tab_button_text_colors;
    kee::transition<float>& tab_active_rect_rel_x;
    kee::transition<float>& exit_button_rect_alpha;

    kee::ui::handle<kee::ui::rect> tab_rect;
    kee::ui::handle<kee::ui::base> tab_display_frame;
    kee::ui::handle<kee::ui::rect> tab_active_rect;
    std::vector<kee::ui::handle<kee::ui::button>> tab_buttons;
    std::vector<kee::ui::handle<kee::ui::text>> tab_button_text;

    kee::ui::handle<kee::ui::button> exit_button;
    kee::ui::handle<kee::ui::rect> exit_button_rect;
    /* TODO: port in icon png */

    compose_tab active_elem;
};

enum class editor::tabs
{
    metadata,
    compose,
    decoration,
    timing
};

} // namespace scene
} // namespace kee