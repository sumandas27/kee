#pragma once

#include "kee/scene/base.hpp"
#include "kee/ui/button.hpp"
#include "kee/ui/image.hpp"

/* TODO: abstract shared info between `editor` and `beatmap` scene */

namespace kee {
namespace scene {

class editor final : public kee::scene::base
{
public:
    editor(const kee::scene::window& window, kee::global_assets& assets);

    void select_key(int id, bool clear);

private:
    void handle_element_events() override;
    void update_element(float dt) override;
    void render_element_behind_children() const override;

    static constexpr float beat_lock_threshold = 0.001f;

    const float music_start_offset;
    const float music_bpm;

    const float beat_step;

    unsigned int id_trans_pause_play_color;
    unsigned int id_trans_pause_play_scale;

    unsigned int id_beat_indicator;
    unsigned int id_beat_ticks_frame;
    unsigned int id_music_slider;
    unsigned int id_pause_play;
    unsigned int id_pause_play_png;
    unsigned int id_music_time_text;
    unsigned int id_key_border;
    unsigned int id_key_frame;
    unsigned int id_editor_key;

    kee::ui::image_texture play_png;
    kee::ui::image_texture pause_png;

    float mouse_wheel_move;

    bool is_music_playing;
    raylib::Music music;

    std::vector<int> selected_key_ids;
};

class editor_key : public kee::ui::button
{
public:
    editor_key(const kee::ui::base::required& reqs, kee::scene::editor& editor_scene, int key_id);

private:
    void handle_element_events() override;

    kee::scene::editor& editor_scene;

    const int key_id;

    bool is_control_clicked;
};

} // namespace scene
} // namespace kee