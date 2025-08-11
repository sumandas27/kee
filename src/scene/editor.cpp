#include "kee/scene/editor.hpp"

#include "kee/ui/button.hpp"

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

    kee::ui::button& pause_play = *dynamic_cast<kee::ui::button*>(child_at(id_pause_play).get());
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
        kee::transition<kee::color>& pause_play_color_trans = *dynamic_cast<kee::transition<kee::color>*>(this->child_at(id_pause_play)->transitions[id_trans_pause_play_color].get());
        kee::transition<float>& pause_play_scale_trans = *dynamic_cast<kee::transition<float>*>(this->child_at(id_pause_play)->transitions[id_trans_pause_play_scale].get());

        pause_play_color_trans.set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
        pause_play_scale_trans.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
    };

    pause_play.on_down = [&]()
    {
        kee::transition<float>& pause_play_scale_trans = *dynamic_cast<kee::transition<float>*>(this->child_at(id_pause_play)->transitions[id_trans_pause_play_scale].get());
        pause_play_scale_trans.set(std::nullopt, 0.9f, 0.5f, kee::transition_type::exp);
    };

    pause_play.on_leave = [&]()
    {
        kee::transition<kee::color>& pause_play_color_trans = *dynamic_cast<kee::transition<kee::color>*>(this->child_at(id_pause_play)->transitions[id_trans_pause_play_color].get());
        kee::transition<float>& pause_play_scale_trans = *dynamic_cast<kee::transition<float>*>(this->child_at(id_pause_play)->transitions[id_trans_pause_play_scale].get());

        pause_play_color_trans.set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
        pause_play_scale_trans.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
    };

    pause_play.on_click = [&]()
    { 
        kee::ui::image& pause_play_img = *dynamic_cast<kee::ui::image*>(this->child_at(id_pause_play)->child_at(id_pause_play_png).get());
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
        kee::ui::button& pause_play = *dynamic_cast<kee::ui::button*>(this->child_at(id_pause_play).get());
        kee::transition<kee::color>& pause_play_color_trans = *dynamic_cast<kee::transition<kee::color>*>(pause_play.transitions[id_trans_pause_play_color].get());
        kee::transition<float>& pause_play_scale_trans = *dynamic_cast<kee::transition<float>*>(pause_play.transitions[id_trans_pause_play_scale].get());
        
        kee::ui::image& pause_play_png = *dynamic_cast<kee::ui::image*>(pause_play.child_at(id_pause_play_png).get());
        pause_play_png.set_color(pause_play_color_trans.get().to_color());
        
        auto& [w, h] = std::get<kee::dims>(pause_play_png.dimensions);
        w.val = pause_play_scale_trans.get();
        h.val = pause_play_scale_trans.get();
    };

    music.SetLooping(false);
    music.SetVolume(0.1f);
    music.Play();
}

void editor::update_element([[maybe_unused]] float dt)
{
    if (music.IsPlaying())
        music.Update();
}

} // namespace scene
} // namespace kee