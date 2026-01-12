#pragma once

#include "kee/scene/editor/compose_tab.hpp"
#include "kee/scene/editor/setup_tab.hpp"
#include "kee/scene/editor/timing_tab.hpp"

/* TODO: if file gets imported and editted before saved, could cause a crash or sm */

namespace kee {
namespace scene {
namespace editor {

class root;
class confirm_exit_ui;

class beatmap_file
{
public:
    beatmap_file(const beatmap_dir_state& dir_state, bool save_metadata_needed);

    beatmap_dir_state dir_state;
    bool save_metadata_needed;
};

class confirm_save_ui : public kee::ui::rect
{
public:
    confirm_save_ui(const kee::ui::required& reqs, const kee::image_texture& error_png, root& root_elem, confirm_exit_ui& render_prio_owner);
    confirm_save_ui(const confirm_save_ui&) = delete;
    confirm_save_ui(confirm_save_ui&&) = delete;
    ~confirm_save_ui();

    confirm_save_ui& operator=(const confirm_save_ui&) = delete;
    confirm_save_ui& operator=(confirm_save_ui&&) = delete;

    bool should_destruct() const;

private:
    bool on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;

    void update_element(float dt) override;

    root& root_elem;
    confirm_exit_ui& render_prio_owner;

    kee::transition<kee::color>& leave_color;
    kee::transition<kee::color>& save_color;

    kee::ui::handle<kee::ui::base> confirm_save_ui_base;

    kee::ui::handle<kee::ui::image> warning_png;
    kee::ui::handle<kee::ui::text> confirm_save_text;

    kee::ui::handle<kee::ui::button> leave_button;
    kee::ui::handle<kee::ui::rect> leave_rect;
    kee::ui::handle<kee::ui::text> leave_text;

    kee::ui::handle<kee::ui::button> save_button;
    kee::ui::handle<kee::ui::rect> save_rect;
    kee::ui::handle<kee::ui::text> save_text;

    bool destruct_flag;
};

class confirm_exit_ui : public kee::ui::base
{
public:
    confirm_exit_ui(const kee::ui::required& reqs, root& root_elem, const kee::image_texture& error_png, float menu_width);

    bool should_destruct() const;

    kee::ui::handle<kee::ui::button> confirm_button;
    std::optional<kee::ui::handle<confirm_save_ui>> confirm_save;

private:
    static constexpr float transition_time = 0.5f;

    bool on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;
    
    void update_element(float dt) override;
    void render_element() const override;

    void queue_for_destruction();

    const kee::image_texture& error_png;

    kee::transition<float>& base_w;
    kee::transition<kee::color>& confirm_button_color;
    kee::transition<kee::color>& go_back_button_color;

    kee::ui::handle<kee::ui::rect> confirm_button_bg;
    kee::ui::text confirm_button_text;

    kee::ui::handle<kee::ui::button> go_back_button;
    kee::ui::handle<kee::ui::rect> go_back_button_bg;
    kee::ui::text go_back_button_text;

    root& root_elem;

    std::optional<float> destroy_timer;
};

class song_ui : public kee::ui::base
{
public:
    song_ui(const kee::ui::required& reqs, const kee::image_texture& arrow_png, const std::filesystem::path& music_path);
    song_ui(const kee::ui::required& reqs, const kee::image_texture& arrow_png, raylib::Music&& music, float music_bpm, float music_start_offset);

    const raylib::Music& get_music() const;
    const std::optional<float>& get_prev_beat() const;

    float get_time() const;
    float get_beat() const;
    void set_beat(float new_beat);

    void pause_play_click(magic_enum::containers::bitset<kee::mods> mods);

    float music_bpm;
    float music_start_offset;

private:
    void update_element(float dt) override;

    static constexpr std::array<float, 6> playback_speeds = { 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f };

    kee::transition<kee::color>& pause_play_color;
    kee::transition<float>& pause_play_scale;
    kee::transition<kee::color>& playback_l_img_color;
    kee::transition<float>& playback_l_img_scale;
    kee::transition<kee::color>& playback_r_img_color;
    kee::transition<float>& playback_r_img_scale;

    kee::ui::handle<kee::ui::slider> music_slider;
    kee::ui::handle<kee::ui::button> pause_play;
    kee::ui::handle<kee::ui::image> pause_play_img;
    kee::ui::handle<kee::ui::text> music_time_text;

    kee::ui::handle<kee::ui::button> playback_l_button;
    kee::ui::handle<kee::ui::rect> playback_l_bg;
    kee::ui::handle<kee::ui::image> playback_l_img;
    
    std::size_t playback_speed_idx;
    kee::ui::handle<kee::ui::rect> playback_text_bg;
    kee::ui::handle<kee::ui::text> playback_text;

    kee::ui::handle<kee::ui::button> playback_r_button;
    kee::ui::handle<kee::ui::rect> playback_r_bg;
    kee::ui::handle<kee::ui::image> playback_r_img;

    raylib::Music music;
    float music_time;

    std::optional<float> last_frame_beat;
    std::optional<float> prev_beat;
};

class editor_hit_object_duration
{
public:
    editor_hit_object_duration(float duration, std::string_view hitsound_name);

    float duration;
    std::string hitsound_name;
};

/**
 * These objects will belong in an `std::set`, whose keys store the `beat` of the object.
 */
class editor_hit_object
{
public:
    editor_hit_object(
        int key, 
        const std::string& hitsound_name, 
        const std::optional<editor_hit_object_duration>& hold_info
    );

    float get_duration() const;

    int key;
    std::string hitsound_name;
    
    std::optional<editor_hit_object_duration> hold_info;
};

/* WISHLIST: refactor save system... replace with magic_enum::bitset maybe ??? */
class beatmap_save_info
{
public:
    beatmap_save_info(bool need_save_metadata, bool need_save_song, bool need_save_img, bool need_save_vid, bool need_save_vid_offset, bool need_save_hit_objs, bool need_save_key_color, bool need_save_hitsound);

    const bool need_save_metadata;
    const bool need_save_song;
    const bool need_save_img;
    const bool need_save_vid;
    const bool need_save_vid_offset;
    const bool need_save_hit_objs;
    const bool need_save_key_color;
    const bool need_save_hitsound;
};

class image_state
{
public:
    image_state(const std::filesystem::path& path);

    std::filesystem::path path;
    kee::image_texture texture;
};

class video_state
{
public:
    video_state(const std::filesystem::path& path, float offset);

    std::filesystem::path path;
    float offset;
};

class key_color_state
{
public:
    key_color_state();
    /**
     * If you already have `boost::json::object` to pass in instead of reparsing.
     */
    key_color_state(const std::filesystem::path& path, const boost::json::object& json);
    key_color_state(const std::filesystem::path& path, root& root_elem);

    std::optional<std::filesystem::path> path;
    std::unordered_map<std::string, std::vector<key_decoration>> decorations;
};

class hitsound_state
{
public:
    hitsound_state();
    /**
     * If you already have a hitsound map to pass in instead of re-validating the directory.
     */
    hitsound_state(const std::filesystem::path& path, std::unordered_map<std::string, raylib::Sound>&& map);
    hitsound_state(const std::filesystem::path& path, root& root_elem);

    std::optional<std::filesystem::path> path;
    std::unordered_map<std::string, raylib::Sound> map; 
};

class root final : public kee::scene::base
{
public:
    root(kee::game& game, kee::global_assets& assets, const std::optional<std::filesystem::path>& beatmap_dir_name);

    std::optional<beatmap_save_info> get_save_info() const;
    bool needs_save(const std::optional<beatmap_save_info>& save_info) const;

    void save();
    void save_existing_beatmap();
    void reset_event_history();

    void set_error(std::string_view error_str, bool from_file_dialog);
    void set_info(std::string_view info_str);
    void set_song(const std::filesystem::path& song_path);

    std::optional<beatmap_file> save_state;

private:
    static constexpr float error_transition_time = 0.5f;

    root(kee::game& game, kee::global_assets& assets, std::optional<beatmap_dir_info> dir_info);

    enum class tabs;

    bool on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;
    void update_element(float dt) override;

    kee::image_texture arrow_png;
    kee::image_texture error_png;
    kee::image_texture exit_png;
    kee::image_texture info_png;

    std::optional<image_state> img_state;
    std::optional<video_state> vid_state;
    key_color_state key_colors;
    hitsound_state hitsounds;

    std::unordered_map<int, std::map<float, editor_hit_object>> hit_objs;
    float approach_beats;

    setup_tab_info setup_info;
    compose_tab_info compose_info;
    timing_tab_info timing_info;

    std::variant<
        kee::ui::handle<setup_tab>,
        kee::ui::handle<compose_tab>,
        kee::ui::handle<timing_tab>
    > active_tab_elem;
    root::tabs active_tab;

    std::vector<std::reference_wrapper<kee::transition<kee::color>>> tab_button_text_colors;
    kee::transition<float>& tab_active_rect_rel_x;
    kee::transition<float>& exit_button_rect_alpha;
    kee::transition<float>& notif_rect_rel_x;
    kee::transition<float>& notif_alpha;

    kee::ui::handle<kee::ui::rect> tab_rect;
    kee::ui::handle<kee::ui::base> tab_display_frame;
    kee::ui::handle<kee::ui::rect> tab_active_rect;
    std::vector<kee::ui::handle<kee::ui::button>> tab_buttons;
    std::vector<kee::ui::handle<kee::ui::text>> tab_button_text;

    kee::ui::handle<kee::ui::button> exit_button;
    kee::ui::handle<kee::ui::rect> exit_button_rect;
    kee::ui::handle<kee::ui::image> exit_button_image;
    std::optional<kee::ui::handle<confirm_exit_ui>> confirm_exit;

    kee::ui::handle<kee::ui::rect> playback_bg;
    kee::ui::handle<kee::ui::base> playback_ui_frame;
    std::variant<
        kee::ui::handle<song_ui>, 
        kee::ui::handle<kee::ui::text>
    > playback_ui;

    kee::ui::handle<kee::ui::rect> notif_rect;
    kee::ui::handle<kee::ui::base> notif_img_frame;
    kee::ui::handle<kee::ui::image> notif_img;
    kee::ui::handle<kee::ui::text> notif_text;

    std::optional<int> notif_skips_before_start;
    float notif_timer;
};

enum class root::tabs
{
    setup,
    compose,
    timing
};

} // namespace editor
} // namespace scene
} // namespace kee