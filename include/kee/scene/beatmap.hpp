#pragma once

#include <deque>

#include "kee/scene/base.hpp"
#include "kee/ui/button.hpp"
#include "kee/ui/image.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/slider.hpp"
#include "kee/ui/text.hpp"
#include "kee/ui/video_player.hpp"

namespace kee {
namespace scene {

/* TODO: half beat forgiveness for ts beatmap */

class beatmap;

class beatmap_hit_object
{
public:
    beatmap_hit_object(float beat, float duration);

    const float beat;
    /**
     * beatmap_hit_objects with duration zero are taps, otherwise they are holds
     */
    const float duration;

    bool hold_is_held;
    bool hold_not_missed;
    bool hold_press_complete;
    std::optional<float> hold_next_combo;
};

class beatmap_key final : public kee::ui::base
{
public:
    beatmap_key(const kee::ui::required& reqs, kee::scene::beatmap& beatmap_scene, int key_id, const raylib::Vector2& relative_pos);

    const std::deque<beatmap_hit_object>& get_hit_objects() const;

    void combo_lose(bool is_miss);

    beatmap_hit_object& front();
    void push(const beatmap_hit_object& object);
    void pop();

private:
    void update_element(float dt) override;
    void render_element() const override;

    kee::scene::beatmap& beatmap_scene;

    kee::transition<float>& combo_lost_alpha;

    std::vector<kee::ui::rect> hit_obj_rects;
    kee::ui::handle<kee::ui::rect> frame;
    kee::ui::handle<kee::ui::rect> frame_combo_lost;
    kee::ui::handle<kee::ui::text> key_text;

    std::deque<beatmap_hit_object> hit_objects;
};

class pause_menu final : public kee::ui::rect
{
public:
    pause_menu(const kee::ui::required& reqs, beatmap& beatmap_scene);

    pause_menu(const pause_menu&) = delete;
    pause_menu(pause_menu&&) = delete;
    ~pause_menu();

    pause_menu& operator=(const pause_menu&) = delete;
    pause_menu& operator=(pause_menu&&) = delete;

    std::optional<bool> destruct_else_restart() const;

private:
    bool on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_key_up(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;

    void update_element(float dt) override;

    beatmap& beatmap_scene;

    kee::transition<float>& ui_rel_y;
    kee::transition<kee::color>& go_back_color;
    kee::transition<kee::color>& restart_color;
    kee::transition<kee::color>& exit_color;

    kee::ui::handle<kee::ui::rect> ui_frame;
    kee::ui::handle<kee::ui::text> pause_menu_text;

    kee::ui::handle<kee::ui::button> go_back_button;
    kee::ui::handle<kee::ui::rect> go_back_rect;
    kee::ui::handle<kee::ui::text> go_back_text;

    kee::ui::handle<kee::ui::button> restart_button;
    kee::ui::handle<kee::ui::rect> restart_rect;
    kee::ui::handle<kee::ui::text> restart_text;

    kee::ui::handle<kee::ui::button> exit_button;
    kee::ui::handle<kee::ui::rect> exit_rect;
    kee::ui::handle<kee::ui::text> exit_text;

    kee::ui::handle<kee::ui::base> game_bg_opacity_frame;
    kee::ui::handle<kee::ui::base> game_bg_opacity_frame_inner;
    kee::ui::handle<kee::ui::text> game_bg_opacity_label;
    kee::ui::handle<kee::ui::text> game_bg_opacity_text;
    std::variant<
        kee::ui::handle<kee::ui::slider>,
        kee::ui::handle<kee::ui::rect>
    > game_bg_opacity_slider;

    std::optional<bool> destruct_else_restart_flag;
};

class end_screen final : public kee::ui::rect
{
public:
    end_screen(const kee::ui::required& reqs, beatmap& beatmap_scene);

private:
    bool on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_key_up(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;

    void update_element(float dt) override;

    void truncate_name_str(kee::ui::handle<kee::ui::text>& name_ui);

    kee::transition<float>& ui_rel_x;
    kee::transition<kee::color>& exit_text_color;

    kee::ui::handle<kee::ui::text> rank_text;
    kee::ui::handle<kee::ui::text> rank_misses_text;

    kee::ui::handle<kee::ui::base> ui_frame;
    kee::ui::handle<kee::ui::text> song_name;
    kee::ui::handle<kee::ui::text> artist_name;
    kee::ui::handle<kee::ui::text> level_name;

    kee::ui::handle<kee::ui::button> exit_button;
    kee::ui::handle<kee::ui::rect> exit_rect;
    kee::ui::handle<kee::ui::text> exit_text;
    kee::ui::handle<kee::ui::text> performance_text;

    kee::ui::handle<kee::ui::base> label_frame;
    kee::ui::handle<kee::ui::text> accuracy_text;
    kee::ui::handle<kee::ui::text> missed_text;
    kee::ui::handle<kee::ui::text> combo_text;
    kee::ui::handle<kee::ui::text> highest_combo_text;

    kee::ui::handle<kee::ui::base> results_frame;
    kee::ui::handle<kee::ui::text> accuracy_result;
    kee::ui::handle<kee::ui::text> missed_result;
    kee::ui::handle<kee::ui::text> combo_result;
    kee::ui::handle<kee::ui::text> highest_combo_result;
};

class beatmap final : public kee::scene::base
{
public:
    beatmap(kee::game& game, kee::global_assets& assets, const std::filesystem::path& beatmap_dir_name);

    float get_beat() const;

    void combo_increment(bool play_sfx);
    void combo_lose(bool is_miss);

    void pause();
    void unpause();
    
    bool has_game_bg() const;
    std::optional<float> get_bg_opacity() const;
    void set_bg_opacity(float opacity);

    const float beat_forgiveness;
    const float approach_beats;

    const std::string song_name;
    const std::string song_artist;
    const std::string mapper;
    const std::string level_name;

    unsigned int max_combo;
    unsigned int prev_total_combo;
    unsigned int prev_highest_combo;
    unsigned int combo;
    unsigned int misses;

private:
    beatmap(kee::game& game, kee::global_assets& assets, beatmap_dir_info&& beatmap_info);

    bool on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_key_up(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;

    void update_element(float dt) override;

    void reset_level();

    const boost::json::object keys_json_obj;
    const float load_time;
    const float music_start_offset;
    const float music_bpm;

    kee::transition<float>& combo_gain;
    kee::transition<float>& end_fade_out_alpha;

    std::optional<kee::image_texture> game_bg_img;
    std::variant<
        std::monostate,
        kee::ui::handle<kee::ui::image>,
        kee::ui::handle<kee::ui::video_player>
    > game_bg;
    std::optional<kee::ui::handle<kee::ui::rect>> load_rect;

    kee::ui::handle<kee::ui::rect> progress_bg;
    kee::ui::handle<kee::ui::rect> progress_rect;

    kee::ui::handle<kee::ui::base> performance_bg;
    kee::ui::handle<kee::ui::base> performance_frame;
    kee::ui::handle<kee::ui::text> accuracy_text;
    kee::ui::handle<kee::ui::text> fc_text;

    kee::ui::handle<kee::ui::text> combo_text;
    kee::ui::handle<kee::ui::text> combo_text_bg;
    kee::ui::handle<kee::ui::base> window_border;
    kee::ui::handle<kee::ui::base> key_frame;

    std::optional<kee::ui::handle<pause_menu>> pause_menu_ui;
    std::optional<kee::ui::handle<kee::ui::rect>> end_fade_out;
    std::optional<kee::ui::handle<end_screen>> end_screen_ui;
    
    std::unordered_map<int, kee::ui::handle<beatmap_key>> keys;
    float end_beat;

    raylib::Music music;
    raylib::Sound hitsound;
    raylib::Sound combo_lost_sfx;

    std::optional<bool> load_time_paused;
    std::optional<float> time_till_end_screen;
    float game_time;
};

} // namespace scene
} // namespace kee