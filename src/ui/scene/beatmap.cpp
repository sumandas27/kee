#include "kee/ui/scene/beatmap.hpp"

#include "kee/ui/key.hpp"
#include "kee/ui/rect.hpp"

namespace kee {
namespace ui {
namespace scene {

beatmap::beatmap(const kee::ui::scene::window& window) :
    kee::ui::scene::base(window),
    load_time(2.0f),
    music_start_offset(0.5f),
    music_bpm(100.0f),
    music("assets/daft-punk-something-about-us/daft-punk-something-about-us.mp3"),
    game_time(0.0f)
{
    id_load_rect = add_child_no_id<kee::ui::rect>(
        raylib::Color(255, 255, 255, 20),
        pos(pos::type::beg, 0),
        pos(pos::type::end, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        std::nullopt, false, 1, false
    );

    id_progress_rect = add_child_no_id<kee::ui::rect>(
        raylib::Color::White(),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 0),
            dim(dim::type::abs, 10)
        ),
        std::nullopt, false, 0, false
    );

    id_window_border = add_child_no_id<kee::ui::base>(
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        dims(
            dim(dim::type::rel, 0.9),
            dim(dim::type::rel, 0.9)
        ),
        true, 0, false
    );

    id_key_frame = child_at(id_window_border)->add_child_no_id<kee::ui::base>(
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        dims(
            dim(dim::type::aspect, 10),
            dim(dim::type::aspect, 4)
        ),
        true, std::nullopt, false
    );

    for (const auto& [id, rel_pos] : key_ui_data)
        child_at(id_window_border)->child_at(id_key_frame)->add_child_with_id<kee::ui::key>(id, id, rel_pos);

    /* TODO: temp just for test */
    end_beat = 4.0f;
}

void beatmap::update_element(float dt)
{
    for (int key = raylib::Keyboard::GetKeyPressed(); key != 0; key = raylib::Keyboard::GetKeyPressed())
    {
        std::unique_ptr<kee::ui::base>& key_frame_ui = child_at(id_window_border)->child_at(id_key_frame);
        if (!key_frame_ui->has_child(key))
            continue;

        kee::ui::key& key_ui = *dynamic_cast<kee::ui::key*>(key_frame_ui->child_at(key).get());
        // TODO: continue
    }

    game_time += dt;
    if (game_time < load_time)
    {
        auto& [_, h] = std::get<kee::ui::dims>(child_at(id_load_rect)->dimensions);
        h.val = (1.0f - game_time / load_time);
    }
    else if (has_child(id_load_rect))
        remove_child(id_load_rect);

    kee::ui::dim& progress_rect_w = std::get<kee::ui::dims>(child_at(id_progress_rect)->dimensions).w;
    progress_rect_w.val = std::clamp(get_beat() / end_beat, 0.0f, 1.0f);

    if (!music.IsPlaying())
    {
        if (game_time >= load_time)
            music.Play();
    }
    else
        music.Update();
}

float beatmap::get_beat() const
{
    return (game_time - load_time - music_start_offset) * music_bpm / 60.0f;
}

} // namespace scene
} // namespace ui
} // namespace kee