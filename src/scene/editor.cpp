#include "kee/scene/editor.hpp"

#include "kee/ui/button.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/slider.hpp"
#include "kee/ui/text.hpp"

namespace kee {
namespace scene {

editor::editor(const kee::scene::window& window) :
    kee::scene::base(window),
    id_trans_pause_play_color(0),
    id_trans_pause_play_scale(1),
    play_png("assets/img/play.png"),
    pause_png("assets/img/pause.png"),
    music("assets/daft-punk-something-about-us/daft-punk-something-about-us.mp3")
{
    id_pause_play = add_child_no_id<kee::ui::button>(
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        dims(
            dim(dim::type::abs, 50),
            dim(dim::type::abs, 50)
        ),
        kee::ui::common(true, std::nullopt, false)
    );

    auto& pause_play = *dynamic_cast<kee::ui::button*>(child_at(id_pause_play).get());
    pause_play.transitions[id_trans_pause_play_color] = std::make_unique<kee::transition<kee::color>>(kee::color::white());
    pause_play.transitions[id_trans_pause_play_scale] = std::make_unique<kee::transition<float>>(1.0f);

    id_pause_play_png = pause_play.add_child_no_id<kee::ui::image>(
        pause_png,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        kee::ui::common(true, std::nullopt, false)
    );

    pause_play.on_hot = [&]()
    {
        std::unordered_map<unsigned int, std::unique_ptr<kee::transition_base>>& transitions = this->child_at(id_pause_play)->transitions;
        auto& color_transition = *dynamic_cast<kee::transition<kee::color>*>(transitions[id_trans_pause_play_color].get());
        auto& scale_transition = *dynamic_cast<kee::transition<float>*>(transitions[id_trans_pause_play_scale].get());

        color_transition.set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
        scale_transition.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
    };

    pause_play.on_down = [&]()
    {
        auto& scale_transition = *dynamic_cast<kee::transition<float>*>(this->child_at(id_pause_play)->transitions[id_trans_pause_play_scale].get());
        scale_transition.set(std::nullopt, 0.9f, 0.5f, kee::transition_type::exp);
    };

    pause_play.on_leave = [&]()
    {
        std::unordered_map<unsigned int, std::unique_ptr<kee::transition_base>>& transitions = this->child_at(id_pause_play)->transitions;
        auto& color_transition = *dynamic_cast<kee::transition<kee::color>*>(transitions[id_trans_pause_play_color].get());
        auto& scale_transition = *dynamic_cast<kee::transition<float>*>(transitions[id_trans_pause_play_scale].get());

        color_transition.set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
        scale_transition.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
    };

    pause_play.on_click = [&]()
    { 
        auto& pause_play_img = *dynamic_cast<kee::ui::image*>(this->child_at(id_pause_play)->child_at(id_pause_play_png).get());
        if (music.IsPlaying())
        {
            music.Pause();
            pause_play_img.set_image(play_png);
        }
        else
        {
            music.Resume();
            pause_play_img.set_image(pause_png);
        }
    };

    pause_play.on_update = [&]([[maybe_unused]] float dt)
    {
        auto& pause_play = *dynamic_cast<kee::ui::button*>(this->child_at(id_pause_play).get());
        auto& pause_play_color_trans = *dynamic_cast<kee::transition<kee::color>*>(pause_play.transitions[id_trans_pause_play_color].get());
        auto& pause_play_scale_trans = *dynamic_cast<kee::transition<float>*>(pause_play.transitions[id_trans_pause_play_scale].get());
        
        auto& pause_play_png = *dynamic_cast<kee::ui::image*>(pause_play.child_at(id_pause_play_png).get());
        pause_play_png.set_color(pause_play_color_trans.get().to_color());
        
        auto& [w, h] = std::get<kee::dims>(pause_play_png.dimensions);
        w.val = pause_play_scale_trans.get();
        h.val = pause_play_scale_trans.get();
    };

    const unsigned int music_length = static_cast<unsigned int>(music.GetTimeLength());
    const std::string music_length_str = std::format("0:00 / {}:{:02}", music_length / 60, music_length % 60);

    id_music_time_text = add_child_no_id<kee::ui::text>(
        raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.75f),
        ui::text_size(ui::text_size::type::abs, 48),
        music_length_str, false,
        kee::ui::common(true, std::nullopt, false)
    );

    id_music_slider = add_child_no_id<kee::ui::slider>(
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.05f),
        dims(
            dim(dim::type::rel, 0.95f),
            dim(dim::type::rel, 0.01f)
        ),
        true, std::nullopt
    );

    add_child_no_id<kee::ui::rect>(
        raylib::Color::Red(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.9f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.1f)
        ),
        std::nullopt,
        kee::ui::rect_roundness(kee::ui::rect_roundness::type::rel_w, 0.5),
        kee::ui::common(true, std::nullopt, false)
    );

    music.SetLooping(false);
    music.SetVolume(0.1f);
    music.Play();
}

void editor::update_element([[maybe_unused]] float dt)
{
    if (music.IsPlaying())
        music.Update();

    const unsigned int music_played = static_cast<unsigned int>(music.GetTimePlayed());
    const unsigned int music_length = static_cast<unsigned int>(music.GetTimeLength());
    const std::string music_time_str = std::format("{}:{:02} / {}:{:02}", music_played / 60, music_played % 60, music_length / 60, music_length % 60);

    auto& music_time_text = *dynamic_cast<kee::ui::text*>(child_at(id_music_time_text).get());
    music_time_text.set_string(music_time_str);

    auto& music_slider = *dynamic_cast<kee::ui::slider*>(child_at(id_music_slider).get());
    music_slider.progress = music.GetTimePlayed() / music.GetTimeLength();
}

} // namespace scene
} // namespace kee