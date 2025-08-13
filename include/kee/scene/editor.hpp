#pragma once

#include "kee/scene/base.hpp"
#include "kee/ui/image.hpp"

namespace kee {
namespace scene {

class editor final : public kee::scene::base
{
public:
    editor(const kee::scene::window& window, kee::global_assets& assets);

private:
    void update_element(float dt) override;

    unsigned int id_trans_pause_play_color;
    unsigned int id_trans_pause_play_scale;

    unsigned int id_pause_play;
    unsigned int id_pause_play_png;
    unsigned int id_music_time_text;
    unsigned int id_music_slider;

    kee::ui::image_texture play_png;
    kee::ui::image_texture pause_png;

    raylib::Music music;
};

} // namespace scene
} // namespace kee