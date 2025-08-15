#pragma once

#include "kee/scene/base.hpp"
#include "kee/ui/image.hpp"

/* TODO: abstract shared info between `editor` and `beatmap` scene */

namespace kee {
namespace scene {

class editor final : public kee::scene::base
{
public:
    editor(const kee::scene::window& window, kee::global_assets& assets);

private:
    void update_element(float dt) override;
    void render_element_behind_children() const override;

    const float music_start_offset;
    const float music_bpm;

    unsigned int id_trans_pause_play_color;
    unsigned int id_trans_pause_play_scale;

    /* TODO: add music player frame base element to resolve active mechanism */
    /* TODO: add red triangle beat indicator */

    unsigned int id_beat_ticks_frame;
    unsigned int id_music_slider;
    unsigned int id_pause_play;
    unsigned int id_pause_play_png;
    unsigned int id_music_time_text;

    kee::ui::image_texture play_png;
    kee::ui::image_texture pause_png;

    bool is_music_playing;
    bool is_music_stopped;
    raylib::Music music;
};

} // namespace scene
} // namespace kee