#pragma once

#include <complex>
#include <future>
#include <span>

#include "kee/scene/base.hpp"
#include "kee/ui/button.hpp"
#include "kee/ui/image.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/scrollable.hpp"
#include "kee/ui/slider.hpp"
#include "kee/ui/text.hpp"

namespace kee {
namespace scene {

class menu;
class play;

class music_analyzer
{
public:
    static constexpr std::size_t bins = 64;

    music_analyzer(const std::filesystem::path& beatmap_dir_path);
    music_analyzer(const music_analyzer&) = delete;
    music_analyzer(music_analyzer&&) = delete;
    ~music_analyzer();

    music_analyzer& operator=(const music_analyzer&) = delete;
    music_analyzer& operator=(music_analyzer&&) = delete;

    const std::filesystem::path& get_beatmap_dir_path() const;

    void update();

    float get_time_length() const;
    float get_time_played() const;
    void seek(float time);

    bool is_playing() const;
    void pause();
    void play();
    void resume();

    void set_volume(float new_volume);

    float get_visualizer_bin(std::size_t i);

private:
    using sample_t = std::int16_t;

    static constexpr unsigned int sample_rate = 48000;
    static constexpr unsigned int bit_depth = 8 * sizeof(sample_t);
    static constexpr unsigned int channels = 2;

    static constexpr std::size_t fft_bins = 8192;
    static constexpr int fft_resolution = 16384;
    static constexpr int frames_per_refresh = 2048;

    static constexpr float smoothing_time_const = 0.75f;
    static constexpr float min_db = -100.f;
    static constexpr float max_db = -33.f;
    static constexpr float inv_db_range = 1.f / (music_analyzer::max_db - music_analyzer::min_db);

    std::filesystem::path beatmap_dir_path;

    raylib::Wave wave;
    std::span<sample_t> samples;
    unsigned int frame_cursor;

    std::array<std::complex<float>, music_analyzer::fft_resolution> fft_work_buffer;
    std::array<float, music_analyzer::fft_resolution> fft_pcm_floats;
    std::array<float, music_analyzer::fft_bins> fft_prev_mags;
    std::array<float, music_analyzer::fft_bins> fft_curr_mags;
    std::array<float, music_analyzer::bins> visualizer_bins;

    /**
     * NOTE: We use raylib's C AudioStream bindings, `raylib-cpp`
     * AudioStream's constructor calls `Unload()` initially, crashing 
     * since no audio stream has previously been loaded in.
     */
    AudioStream audio_stream;
};

class opening_transitions
{
public:
    opening_transitions(menu& menu_scene);

    kee::transition<float>& k_rect_alpha;
    kee::transition<float>& k_rect_x;

    kee::transition<float>& e1_text_alpha;
    kee::transition<float>& e1_rect_alpha;

    kee::transition<float>& e2_text_alpha;
    kee::transition<float>& e2_rect_alpha;
    kee::transition<float>& e2_rect_x;
};

class music_transitions
{
public:
    music_transitions(menu& menu_scene);

    menu& menu_scene;

    kee::transition<float>& slider_alpha;
    kee::transition<float>& slider_width;

    kee::transition<kee::color>& pause_play_color;
    kee::transition<kee::color>& step_l_color;
    kee::transition<kee::color>& step_r_color;
    kee::transition<kee::color>& setting_color;
    kee::transition<kee::color>& exit_color;

    kee::transition<float>& pause_play_border;
    kee::transition<float>& step_l_border;
    kee::transition<float>& step_r_border;
    kee::transition<float>& setting_border;
    kee::transition<float>& exit_border;
    kee::transition<float>& song_ui_alpha;

    raylib::Image step_texture;
    raylib::Image setting_texture;

    kee::ui::handle<kee::ui::slider> music_slider;
    kee::ui::handle<kee::ui::button> pause_play;
    kee::ui::handle<kee::ui::image> pause_play_img;
    kee::ui::handle<kee::ui::button> step_l;
    kee::ui::handle<kee::ui::image> step_l_img;
    kee::ui::handle<kee::ui::button> step_r;
    kee::ui::handle<kee::ui::image> step_r_img;

    kee::ui::handle<kee::ui::base> setting_exit_frame;
    kee::ui::handle<kee::ui::button> setting_button;
    kee::ui::handle<kee::ui::image> setting_img;
    kee::ui::handle<kee::ui::button> exit_button;
    kee::ui::handle<kee::ui::image> exit_img;
};

class level_ui_assets
{
public:
    level_ui_assets(const std::filesystem::path& beatmap_dir_path);

    const std::filesystem::path beatmap_dir_path;

    std::optional<raylib::Image> img;

    std::string song_name;
    std::string song_artist;
    std::string mapper;
    std::string level_name;
};

class level_ui final : public kee::ui::button
{
public:
    level_ui(
        const kee::ui::required& reqs,
        const kee::pos& x,
        const kee::pos& y,
        const std::variant<kee::dims, kee::border>& dims,
        bool centered,
        bool is_selected,
        play& play_ui,
        const level_ui_assets& ui_assets,
        std::size_t idx
    );

    void select();
    void unselect();

private:
    void update_element(float dt) override;

    const std::size_t idx;

    play& play_ui;

    kee::transition<kee::color>& frame_color;
    kee::transition<kee::color>& image_frame_color;

    kee::ui::handle<kee::ui::rect> frame;

    kee::ui::handle<kee::ui::rect> image_frame;
    std::optional<kee::ui::handle<kee::ui::image>> image;

    kee::ui::handle<kee::ui::base> text_frame;
    kee::ui::handle<kee::ui::base> text_inner_frame;
    kee::ui::handle<kee::ui::text> song_name_text;
    kee::ui::handle<kee::ui::text> song_artist_text; /* TODO: clamp these */
    kee::ui::handle<kee::ui::text> level_name_text;

    bool is_selected;
};

/**
 * NOTE: Inherits button so this consumes all mouse inputs, nothing in the
 * background gets interacted with.
 */
class play final : public kee::ui::button
{
public:
    play(const kee::ui::required& reqs, menu& menu_scene, const std::filesystem::path& music_analyzer_beatmap_dir);

    void set_selected_level(std::size_t idx);

private:
    void update_element(float dt) override;

    const raylib::Image search_png;

    kee::transition<kee::color>& back_rect_color;

    kee::ui::handle<kee::ui::base> top_bar_frame;

    kee::ui::handle<kee::ui::button> back_button;
    kee::ui::handle<kee::ui::rect> back_rect;
    kee::ui::handle<kee::ui::image> back_image;

    /* TODO: will move to being a textbox */
    kee::ui::handle<kee::ui::rect> search_bar;
    kee::ui::handle<kee::ui::button> search_button;
    kee::ui::handle<kee::ui::rect> search_rect;
    kee::ui::handle<kee::ui::image> search_image;

    kee::ui::handle<kee::ui::scrollable> level_list_scrollable;
    kee::ui::handle<kee::ui::base> level_list_inner;
    std::vector<kee::ui::handle<level_ui>> level_list;

    kee::ui::handle<kee::ui::rect> selected_bg;
    kee::ui::handle<kee::ui::base> selected_frame;
    
    kee::ui::handle<kee::ui::rect> selected_image_frame;
    std::optional<kee::ui::handle<kee::ui::image>> selected_image;

    kee::ui::handle<kee::ui::base> selected_text_frame;
    kee::ui::handle<kee::ui::text> selected_song_name_text;
    kee::ui::handle<kee::ui::text> selected_song_artist_text; /* TODO: clamp these */
    kee::ui::handle<kee::ui::text> selected_level_name_text; /* TODO NEXT: work on ts */

    /**
     * `level_list` is never pushed to or popped from, meaning
     * indexing is a valid pointer.
     */
    std::size_t level_list_selected_idx;
};

/* WISHLIST: make bg more interesting, two ideas:
    1. default bg img maybe (responding to mouse movement like it does in osu)
    1. particle system (from vis.js) with music visualizer
*/

class menu final : public kee::scene::base
{
public:
    menu(const kee::scene::required& reqs, const beatmap_dir_info& beatmap_info, bool from_game_init);

private:
    /* TODO: `friend` is kind of a bad workaround, think about ts */
    friend class music_transitions;
    friend class play;

    void update_element(float dt) override;

    kee::transition<float>& k_text_alpha;
    std::optional<opening_transitions> opening_trns;
    std::optional<music_transitions> music_trns;

    kee::transition<float>& k_scale;
    kee::transition<float>& e1_scale;
    kee::transition<float>& e2_scale;

    kee::transition<float>& edit_text_alpha;
    kee::transition<float>& play_text_alpha;
    kee::transition<float>& browse_text_alpha;

    std::future<std::vector<level_ui_assets>> play_assets_future;
    std::vector<level_ui_assets> play_assets;

    kee::ui::handle<kee::ui::button> k_button;
    kee::ui::handle<kee::ui::rect> k_rect;
    kee::ui::handle<kee::ui::text> k_text;
    kee::ui::handle<kee::ui::text> edit_text;

    kee::ui::handle<kee::ui::button> e1_button;
    kee::ui::handle<kee::ui::rect> e1_rect;
    kee::ui::handle<kee::ui::text> e1_text;
    kee::ui::handle<kee::ui::text> play_text;

    kee::ui::handle<kee::ui::button> e2_button;
    kee::ui::handle<kee::ui::rect> e2_rect;
    kee::ui::handle<kee::ui::text> e2_text;
    kee::ui::handle<kee::ui::text> browse_text;

    std::optional<raylib::Image> music_cover_art_texture;

    music_analyzer analyzer;
    float music_time;

    kee::ui::handle<kee::ui::base> song_ui_frame_outer;
    kee::ui::handle<kee::ui::base> song_ui_frame_inner;
    kee::ui::handle<kee::ui::rect> music_cover_art_frame;
    std::optional<kee::ui::handle<kee::ui::image>> music_cover_art;

    kee::ui::handle<kee::ui::base> music_info_text_frame;
    kee::ui::handle<kee::ui::text> music_name_text;
    kee::ui::handle<kee::ui::text> music_artist_text;
    kee::ui::handle<kee::ui::text> music_time_text;

    std::vector<kee::ui::handle<kee::ui::rect>> visualizer_bot;
    std::vector<kee::ui::handle<kee::ui::rect>> visualizer_top;

    std::optional<kee::ui::handle<play>> play_ui;

    float scene_time;
};

} // namespace scene
} // namespace kee