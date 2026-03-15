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

class beatmap;

class beatmap_hit_object_hold
{
public:
    beatmap_hit_object_hold(float beat, float duration, std::string_view end_hitsound);

    const float duration;
    const std::string hitsound;

    bool is_held;
    bool not_missed;
    bool press_complete;
    std::optional<float> next_combo;
};

class beatmap_hit_object
{
public:
    beatmap_hit_object(float beat, std::string_view start_hitsound);
    beatmap_hit_object(float beat, std::string_view start_hitsound, float duration, std::string_view end_hitsound);

    float get_end_beat() const;

    const float beat;
    const std::string hitsound;
    
    std::optional<beatmap_hit_object_hold> hold;
};

class beatmap_key final : public kee::ui::base
{
public:
    beatmap_key(const kee::ui::required& reqs, kee::scene::beatmap& beatmap_scene, int key_id, const raylib::Vector2& relative_pos, const std::vector<key_decoration>& key_colors);

    const std::deque<beatmap_hit_object>& get_hit_objects() const;

    void combo_lose(bool is_miss, float lost_beat);

    beatmap_hit_object& front();
    void push(const beatmap_hit_object& object);
    void pop();

    bool is_down;

private:
    void update_element(float dt) override;
    void render_element() const override;

    kee::scene::beatmap& beatmap_scene;
    kee::transition<float>& combo_lost_alpha;

    const std::vector<key_decoration> key_colors;

    std::vector<kee::ui::rect> hit_obj_rects;
    kee::ui::handle<kee::ui::rect> frame;
    kee::ui::handle<kee::ui::rect> frame_combo_lost;
    kee::ui::handle<kee::ui::text> key_text;

    std::deque<beatmap_hit_object> hit_objects;
    std::size_t key_colors_idx;
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
    end_screen(const end_screen&) = delete;
    end_screen(end_screen&&) = delete;
    ~end_screen();

    end_screen& operator=(const end_screen&) = delete;
    end_screen& operator=(end_screen&&) = delete;

    bool should_reset() const;

private:
    static constexpr float result_text_size = 0.12f;

    bool on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_key_up(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;

    void update_element(float dt) override;

    beatmap& beatmap_scene;

    kee::transition<float>& ui_rel_x;
    kee::transition<kee::color>& leave_color;
    kee::transition<kee::color>& restart_color;

    kee::ui::handle<kee::ui::base> level_frame;
    kee::ui::handle<kee::ui::base> display_outer_frame;
    kee::ui::handle<kee::ui::rect> display_frame;
    kee::ui::handle<kee::ui::rect> image_frame;
    std::optional<kee::ui::handle<kee::ui::image>> image;

    kee::ui::handle<kee::ui::base> text_frame;
    kee::ui::handle<kee::ui::base> text_inner_frame;
    kee::ui::handle<kee::ui::text> song_name_text;
    kee::ui::handle<kee::ui::text> song_artist_text; /* TODO: clamp these */
    kee::ui::handle<kee::ui::text> level_name_text;

    kee::ui::handle<kee::ui::base> ui_frame;
    kee::ui::handle<kee::ui::base> ui_inner_frame;
    kee::ui::handle<kee::ui::base> performance_outer_frame;
    kee::ui::handle<kee::ui::base> performance_frame;

    kee::ui::handle<kee::ui::rect> rating_rect;
    kee::ui::handle<kee::ui::base> rating_frame;
    kee::ui::handle<kee::ui::image> rating_star_img;
    kee::ui::handle<kee::ui::text> rating_text;

    kee::ui::handle<kee::ui::base> high_score_text_frame;
    kee::ui::handle<kee::ui::text> high_score_text;

    kee::ui::handle<kee::ui::button> leave_button;
    kee::ui::handle<kee::ui::image> leave_img;
    kee::ui::handle<kee::ui::text> leave_text;

    kee::ui::handle<kee::ui::button> restart_button;
    kee::ui::handle<kee::ui::image> restart_img;
    kee::ui::handle<kee::ui::text> restart_text;

    kee::ui::handle<kee::ui::base> label_frame;
    kee::ui::handle<kee::ui::text> score_text;
    kee::ui::handle<kee::ui::text> accuracy_text;
    kee::ui::handle<kee::ui::text> missed_text;
    kee::ui::handle<kee::ui::text> combo_text;
    kee::ui::handle<kee::ui::text> highest_combo_text;
    kee::ui::handle<kee::ui::text> attempts_text;

    kee::ui::handle<kee::ui::text> score_result;
    kee::ui::handle<kee::ui::text> accuracy_result;
    kee::ui::handle<kee::ui::text> missed_result;
    kee::ui::handle<kee::ui::text> combo_result;
    kee::ui::handle<kee::ui::text> highest_combo_result;
    kee::ui::handle<kee::ui::text> attempts_result;

    bool should_reset_flag;
};

class beatmap final : public kee::scene::base
{
public:
    static constexpr unsigned int score_fc = 100;

    beatmap(const kee::scene::required& reqs, std::size_t beatmap_id, beatmap_dir_info&& beatmap_info);

    float get_beat() const;

    void combo_increment(const std::optional<std::string>& hitsound_name);
    void combo_lose(bool is_miss, float lost_beat);

    void pause();
    void unpause();
    
    bool has_game_bg() const;
    std::optional<float> get_bg_opacity() const;
    void set_bg_opacity(float opacity);

    void update_performance();
    void reset_level();

    const raylib::Image leave_png;
    const raylib::Image restart_png;

    const float beat_forgiveness;
    const float approach_beats;

    const std::optional<raylib::Image> level_img;
    const std::string song_name;
    const std::string song_artist;
    const std::string mapper;
    const std::string level_name;
    const unsigned int metadata_total_combo;

    const std::size_t beatmap_id;

    std::optional<unsigned int> score;
    unsigned int total_combo;
    unsigned int prev_accumulated_combo;
    unsigned int prev_highest_combo;
    unsigned int combo;
    unsigned int misses;

    std::optional<performance_stats> best_performance;
    unsigned int total_attempts;
    unsigned int session_attempts;

private:
    static constexpr float load_time = 2.f;
    static constexpr float attempts_text_full_alpha_time = 1.25f;
    static constexpr float attempts_text_fade_out_time = 0.75f;

    bool on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_key_up(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;

    void update_element(float dt) override;

    const boost::json::object keys_json_obj;
    const std::optional<boost::json::object> key_color_json_obj;

    const std::optional<float> video_offset;
    const float music_start_offset;
    const float music_bpm;

    kee::transition<float>& combo_gain;
    kee::transition<float>& end_fade_out_alpha;
    kee::transition<float>& session_attempts_text_alpha;

    std::optional<raylib::Image> game_bg_img;
    std::variant<
        std::monostate,
        kee::ui::handle<kee::ui::image>,
        kee::ui::handle<kee::ui::video_player>
    > game_bg;
    std::optional<kee::ui::handle<kee::ui::rect>> load_rect;
    std::optional<kee::ui::handle<kee::ui::text>> session_attempts_text;

    kee::ui::handle<kee::ui::rect> progress_bg;
    kee::ui::handle<kee::ui::rect> progress_rect;

    kee::ui::handle<kee::ui::base> performance_bg;
    kee::ui::handle<kee::ui::base> performance_frame;
    kee::ui::handle<kee::ui::text> accuracy_text;
    kee::ui::handle<kee::ui::text> score_text;
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
    raylib::Sound combo_lost_sfx;
    std::unordered_map<std::string, raylib::Sound> hitsounds;

    std::optional<bool> load_time_paused;
    bool has_attempt_text_fade_out_started;

    std::optional<float> time_till_end_screen;
    float game_time;
};

} // namespace scene
} // namespace kee