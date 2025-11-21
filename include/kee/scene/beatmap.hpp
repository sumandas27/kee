#pragma once

#include <deque>

#include "kee/scene/base.hpp"
#include "kee/ui/button.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/text.hpp"

namespace kee {
namespace scene {

/* TODO: cap misses at one for holds */
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

    float combo_lost_time;
    std::deque<beatmap_hit_object> hit_objects;
};

class pause_menu final : public kee::ui::rect
{
public:
    pause_menu(const kee::ui::required& reqs, std::optional<bool>& load_time_paused, raylib::Music& music);
    pause_menu(const pause_menu&) = delete;
    pause_menu(pause_menu&&) = delete;
    ~pause_menu();

    pause_menu& operator=(const pause_menu&) = delete;
    pause_menu& operator=(pause_menu&&) = delete;

    bool should_destruct() const;

private:
    bool on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_key_up(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;

    void update_element(float dt) override;

    std::optional<bool>& load_time_paused;
    raylib::Music& music;

    kee::transition<float>& ui_rel_y;
    kee::transition<kee::color>& go_back_color;
    kee::transition<kee::color>& exit_color;

    kee::ui::handle<kee::ui::rect> ui_frame;
    kee::ui::handle<kee::ui::text> pause_menu_text;

    kee::ui::handle<kee::ui::button> go_back_button;
    kee::ui::handle<kee::ui::rect> go_back_rect;
    kee::ui::handle<kee::ui::text> go_back_text;

    kee::ui::handle<kee::ui::button> exit_button;
    kee::ui::handle<kee::ui::rect> exit_rect;
    kee::ui::handle<kee::ui::text> exit_text;

    bool destruct_flag;
};

class beatmap final : public kee::scene::base
{
public:
    beatmap(const kee::scene::window& window, kee::game& game, kee::global_assets& assets, const std::filesystem::path& beatmap_dir_name);

    float get_beat() const;

    void combo_increment(bool play_sfx);
    void combo_lose(bool is_miss);

    const float beat_forgiveness;
    const float approach_beats;

    unsigned int max_combo;

private:
    beatmap(const kee::scene::window& window, kee::game& game, kee::global_assets& assets, beatmap_dir_info&& beatmap_info);

    bool on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_key_up(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;

    void update_element(float dt) override;

    kee::transition<float>& combo_gain;

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

    const float load_time;
    const float music_start_offset;
    const float music_bpm;

    std::unordered_map<int, kee::ui::handle<beatmap_key>> keys;

    raylib::Music music;
    raylib::Sound hitsound;
    raylib::Sound combo_lost_sfx;

    unsigned int prev_total_combo;
    unsigned int combo;
    unsigned int misses;

    float combo_time;
    float end_beat;

    std::optional<bool> load_time_paused;
    float game_time;
};

} // namespace scene
} // namespace kee