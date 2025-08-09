#pragma once

#include "kee/scene/base.hpp"
#include "kee/ui/key.hpp"

namespace kee {
namespace scene {

class beatmap final : public kee::scene::base
{
public:
    beatmap(const kee::scene::window& window);

    float get_beat() const;

    void combo_increment_with_sound();
    void combo_increment_no_sound();
    void combo_lose();

    const float input_tolerance;
    const float approach_beats;

private:
    void update_element(float dt) override;

    kee::ui::key& get_key(int keycode);

    const float load_time;
    const float max_combo_time;

    const float music_start_offset;
    const float music_bpm;

    const unsigned int id_trans_combo_gain;

    unsigned int id_load_rect;
    unsigned int id_progress_rect;
    unsigned int id_combo_text;
    unsigned int id_combo_text_bg;
    unsigned int id_window_border;
    unsigned int id_key_frame;

    raylib::Music music;
    raylib::Sound hitsound;
    raylib::Sound combo_lost_sfx;

    unsigned int combo;
    float combo_time;
    float end_beat;

    float game_time;
};

} // namespace scene
} // namespace kee