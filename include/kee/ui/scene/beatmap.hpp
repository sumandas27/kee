#pragma once

#include "kee/ui/scene/base.hpp"

namespace kee {
namespace ui {
namespace scene {

class beatmap final : public kee::ui::scene::base
{
public:
    beatmap(const kee::ui::scene::window& window);

private:
    void update_element(float dt) override;

    float get_beat() const;

    const float load_time;

    const float music_start_offset;
    const float music_bpm;

    unsigned int id_load_rect;
    unsigned int id_progress_rect;
    unsigned int id_window_border;
    unsigned int id_key_frame;

    raylib::Music music;

    float end_beat;

    float game_time;
};

} // namespace scene
} // namespace ui
} // namespace kee