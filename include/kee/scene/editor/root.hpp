#pragma once

#include "kee/scene/editor/compose_tab.hpp"
#include "kee/scene/editor/decoration_tab.hpp"
#include "kee/scene/editor/setup_tab.hpp"
#include "kee/scene/editor/timing_tab.hpp"

namespace kee {
namespace scene {
namespace editor {

class root final : public kee::scene::base
{
public:
    root(const kee::scene::window& window, kee::game& game, kee::global_assets& assets);

    void set_error(std::string_view error_str, bool from_file_dialog);

    float get_beat() const;
    void set_beat(float new_beat);

private:
    static constexpr std::array<float, 6> playback_speeds = { 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f };
    static constexpr float error_transition_time = 0.5f;

    enum class tabs;

    bool on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;
    void update_element(float dt) override;

    const float music_start_offset;
    const float music_bpm;

    kee::image_texture exit_png;
    kee::image_texture error_png;
    kee::image_texture pause_png;
    kee::image_texture arrow_png;

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
    kee::transition<kee::color>& pause_play_color;
    kee::transition<float>& pause_play_scale;
    kee::transition<kee::color>& playback_l_img_color;
    kee::transition<float>& playback_l_img_scale;
    kee::transition<kee::color>& playback_r_img_color;
    kee::transition<float>& playback_r_img_scale;
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

    kee::ui::handle<kee::ui::rect> playback_bg;
    kee::ui::handle<kee::ui::base> playback_ui_frame;
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

    kee::ui::handle<kee::ui::rect> error_rect;
    kee::ui::handle<kee::ui::base> error_img_frame;
    kee::ui::handle<kee::ui::image> error_img;
    kee::ui::handle<kee::ui::text> error_text;

    raylib::Music music;
    float music_time;
    std::optional<float> prev_beat;

    std::optional<int> error_skips_before_start;
    float error_timer;

    compose_tab_info compose_info;
    timing_tab_info timing_info;
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