#pragma once

#include "kee/scene/editor/compose_tab.hpp"
#include "kee/scene/editor/decoration_tab.hpp"
#include "kee/scene/editor/setup_tab.hpp"
#include "kee/scene/editor/timing_tab.hpp"

namespace kee {
namespace scene {
namespace editor {

class confirm_exit_ui : public kee::ui::base
{
public:
    confirm_exit_ui(const kee::ui::required& reqs, float menu_width);

    bool should_destruct() const;

private:
    static constexpr float transition_time = 0.5f;

    bool on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;
    
    void update_element(float dt) override;
    void render_element() const override;

    void queue_for_destruction();

    const float menu_width;

    kee::transition<float>& base_w;
    kee::transition<kee::color>& confirm_button_text_color;
    kee::transition<kee::color>& go_back_button_text_color;

    kee::ui::handle<kee::ui::button> confirm_button;
    kee::ui::handle<kee::ui::rect> confirm_button_bg;
    kee::ui::text confirm_button_text;

    kee::ui::handle<kee::ui::button> go_back_button;
    kee::ui::handle<kee::ui::rect> go_back_button_bg;
    kee::ui::text go_back_button_text;

    std::optional<float> destroy_timer;
};

class song_ui : public kee::ui::base
{
public:
    song_ui(
        const kee::ui::required& reqs, 
        const kee::image_texture& arrow_png, 
        const std::filesystem::path& music_path
    );

    const raylib::Music& get_music() const;
    const std::optional<float>& get_prev_beat() const;

    float get_beat() const;
    void set_beat(float new_beat);

    void pause_play_click(magic_enum::containers::bitset<kee::mods> mods);

    /**
     * TODO: when file writing is implemented, change this to bpm 100 offset 0.43
     */
    float music_bpm;
    float music_start_offset;

private:
    void update_element(float dt) override;

    static constexpr std::array<float, 6> playback_speeds = { 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f };

    const kee::image_texture& arrow_png;

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

class root final : public kee::scene::base
{
public:
    root(const kee::scene::window& window, kee::game& game, kee::global_assets& assets);

    void set_error(std::string_view error_str, bool from_file_dialog);
    void set_song(const std::filesystem::path& song_path);

private:
    static constexpr float error_transition_time = 0.5f;

    enum class tabs;

    bool on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;
    void update_element(float dt) override;

    kee::image_texture exit_png;
    kee::image_texture error_png;
    kee::image_texture arrow_png;

    float approach_beats;

    setup_tab_info setup_info;
    compose_tab_info compose_info;
    timing_tab_info timing_info;

    std::variant<
        kee::ui::handle<setup_tab>,
        kee::ui::handle<compose_tab>,
        kee::ui::handle<decoration_tab>,
        kee::ui::handle<timing_tab>
    > active_tab_elem;
    root::tabs active_tab;

    std::vector<std::reference_wrapper<kee::transition<kee::color>>> tab_button_text_colors;
    kee::transition<float>& tab_active_rect_rel_x;
    kee::transition<float>& exit_button_rect_alpha;
    kee::transition<float>& error_rect_rel_x;
    kee::transition<float>& error_alpha;

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

    kee::ui::handle<kee::ui::rect> error_rect;
    kee::ui::handle<kee::ui::base> error_img_frame;
    kee::ui::handle<kee::ui::image> error_img;
    kee::ui::handle<kee::ui::text> error_text;

    std::optional<int> error_skips_before_start;
    float error_timer;
};

enum class root::tabs
{
    setup,
    compose,
    decoration,
    timing
};

} // namespace editor
} // namespace scene
} // namespace kee