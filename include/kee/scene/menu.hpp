#pragma once

#include "kee/scene/base.hpp"
#include "kee/ui/button.hpp"
#include "kee/ui/image.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/slider.hpp"
#include "kee/ui/text.hpp"

namespace kee {
namespace scene {

class menu;

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

    kee::transition<float>& music_volume_multiplier;
    bool music_volume_trns_finished;

    kee::transition<kee::color>& pause_play_color;
    kee::transition<float>& pause_play_border;
    kee::transition<float>& song_ui_alpha;

    kee::image_texture step_texture;

    kee::ui::handle<kee::ui::slider> music_slider;
    kee::ui::handle<kee::ui::button> pause_play;
    kee::ui::handle<kee::ui::image> pause_play_img;
    kee::ui::handle<kee::ui::button> step_l;
    kee::ui::handle<kee::ui::image> step_l_img;
    kee::ui::handle<kee::ui::button> step_r;
    kee::ui::handle<kee::ui::image> step_r_img;
};

class menu final : public kee::scene::base
{
public:
    menu(kee::game& game, kee::global_assets& assets, beatmap_dir_info&& beatmap_info);

private:
    friend class music_transitions;

    void update_element(float dt) override;

    kee::transition<float>& k_text_alpha;
    std::optional<opening_transitions> opening_trns;
    std::optional<music_transitions> music_trns;

    kee::ui::handle<kee::ui::rect> k_rect;
    kee::ui::handle<kee::ui::text> k_text;

    kee::ui::handle<kee::ui::rect> e1_rect;
    kee::ui::handle<kee::ui::text> e1_text;

    kee::ui::handle<kee::ui::rect> e2_rect;
    kee::ui::handle<kee::ui::text> e2_text;

    std::optional<kee::image_texture> music_cover_art_texture;
    raylib::Music music;
    float music_time;

    kee::ui::handle<kee::ui::base> song_ui_frame_outer;
    kee::ui::handle<kee::ui::base> song_ui_frame_inner;
    kee::ui::handle<kee::ui::rect> music_cover_art_frame;
    kee::ui::handle<kee::ui::image> music_cover_art;
    kee::ui::handle<kee::ui::base> music_info_text_frame;
    kee::ui::handle<kee::ui::text> music_name_text;
    kee::ui::handle<kee::ui::text> music_artist_text;
    kee::ui::handle<kee::ui::text> music_time_text;

    float scene_time;
};

} // namespace scene
} // namespace kee