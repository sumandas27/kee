#include "kee/scene/editor.hpp"

#include "kee/ui/button.hpp"

namespace kee {
namespace scene {

editor::editor(const kee::scene::window& window) :
    kee::scene::base(window),
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
        true, std::nullopt, false
    );

    kee::ui::button& pause_play = *dynamic_cast<kee::ui::button*>(child_at(id_pause_play).get());
    //pause_play.transitions[id_trans_pause_play_color] = std::make_unique<kee::transition<raylib::Vector4>>(raylib::Vector4(255, 255, 255, 255));

    id_pause_play_png = pause_play.add_child_no_id<kee::ui::image>(
        pause_png,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        true, std::nullopt, false
    );

    /*pause_play.on_hot = [&]()
    {
        kee::transition<raylib::Vector4>& pause_play_color_trans = *dynamic_cast<kee::transition<raylib::Vector4>*>(this->child_at(id_pause_play)->transitions[id_trans_pause_play_color].get());
        pause_play_color_trans.set(std::nullopt, raylib::Vector4(255, 140, 0, 255), 0.25f, kee::transition_type::exp);
    };

    pause_play.on_down = [&]()
    {
        kee::transition<raylib::Vector4>& pause_play_color_trans = *dynamic_cast<kee::transition<raylib::Vector4>*>(this->child_at(id_pause_play)->transitions[id_trans_pause_play_color].get());
        pause_play_color_trans.set(std::nullopt, raylib::Vector4(255, 0, 0, 255), 0.25f, kee::transition_type::exp);
    };

    pause_play.on_leave = [&]()
    {
        kee::transition<raylib::Vector4>& pause_play_color_trans = *dynamic_cast<kee::transition<raylib::Vector4>*>(this->child_at(id_pause_play)->transitions[id_trans_pause_play_color].get());
        pause_play_color_trans.set(std::nullopt, raylib::Vector4(255, 255, 255, 255), 0.25f, kee::transition_type::exp);
    };*/

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

    /*pause_play.on_update = [&]([[maybe_unused]] float dt)
    {
        kee::ui::button& pause_play = *dynamic_cast<kee::ui::button*>(this->child_at(id_pause_play).get());
        kee::transition<raylib::Vector4>& pause_play_color_trans = *dynamic_cast<kee::transition<raylib::Vector4>*>(pause_play.transitions[id_trans_pause_play_color].get());

        const raylib::Vector4 pause_play_color = pause_play_color_trans.get();
        pause_play.set_color(raylib::Color(
            static_cast<unsigned char>(pause_play_color.x),
            static_cast<unsigned char>(pause_play_color.y),
            static_cast<unsigned char>(pause_play_color.z),
            static_cast<unsigned char>(pause_play_color.w)
        ));
    };*/

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