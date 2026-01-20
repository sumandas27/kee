#include "kee/scene/menu.hpp"

namespace kee {
namespace scene {

opening_transitions::opening_transitions(menu& menu_scene) :
    k_rect_alpha(menu_scene.add_transition<float>(0.0f)),
    k_rect_x(menu_scene.add_transition<float>(0.5f)),
    e1_text_alpha(menu_scene.add_transition<float>(0.0f)),
    e1_rect_alpha(menu_scene.add_transition<float>(0.0f)),
    e2_text_alpha(menu_scene.add_transition<float>(0.0f)),
    e2_rect_alpha(menu_scene.add_transition<float>(0.0f)),
    e2_rect_x(menu_scene.add_transition<float>(0.5f))
{
    k_rect_alpha.set(std::nullopt, 255.0f, 0.5f, kee::transition_type::exp);
    k_rect_x.set(std::nullopt, 0.25f, 0.5f, kee::transition_type::exp);
    e1_text_alpha.set(std::nullopt, 255.0f, 0.5f, kee::transition_type::exp);
    e1_rect_alpha.set(std::nullopt, 255.0f, 0.5f, kee::transition_type::exp);
    e2_text_alpha.set(std::nullopt, 255.0f, 0.5f, kee::transition_type::exp);
    e2_rect_alpha.set(std::nullopt, 255.0f, 0.5f, kee::transition_type::exp);
    e2_rect_x.set(std::nullopt, 0.75f, 0.5f, kee::transition_type::exp);
}

music_transitions::music_transitions(menu& menu_scene) :
    menu_scene(menu_scene),
    slider_alpha(menu_scene.add_transition<float>(0.0f)),
    slider_width(menu_scene.add_transition<float>(0.0f)),
    music_volume_multiplier(menu_scene.add_transition<float>(0.0f)),
    music_volume_trns_finished(false),
    pause_play_color(menu_scene.add_transition<kee::color>(kee::color(255, 255, 255, 0))),
    pause_play_scale(menu_scene.add_transition<float>(1.0f)),
    music_slider(menu_scene.add_child<kee::ui::slider>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.9f),
        dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::rel, 0.01f)
        ),
        true, true
    )),
    pause_play(menu_scene.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.95f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.04f)
        ),
        true
    )),
    pause_play_img(pause_play.ref.add_child<kee::ui::image>(std::nullopt,
        menu_scene.assets.play_png, pause_play_color.get(),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    ))
{
    slider_alpha.set(std::nullopt, 255.0f, 0.5f, kee::transition_type::lin);
    slider_width.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
    music_volume_multiplier.set(std::nullopt, 2.0f, 2.0f, kee::transition_type::inv_exp);
    pause_play_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::lin);

    music_slider.ref.on_event = [&, music_is_playing = this->menu_scene.music.IsPlaying()](ui::slider::event slider_event) mutable
    {
        switch (slider_event)
        {
        case ui::slider::event::on_down:
            music_is_playing = this->menu_scene.music.IsPlaying();
            this->menu_scene.music.Pause();
            break;
        case ui::slider::event::on_release:
            this->menu_scene.music.Seek(this->music_slider.ref.progress * this->menu_scene.music.GetTimeLength());
            if (music_is_playing)
                this->menu_scene.music.Resume();
            break;
        default:
            break;
        }
    };

    pause_play.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            pause_play_color.set(std::nullopt, kee::color(255, 255, 255, 200), 0.5f, kee::transition_type::exp);
            pause_play_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            pause_play_scale.set(std::nullopt, 0.9f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            pause_play_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
            pause_play_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    pause_play.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    { 
        if (this->menu_scene.music.IsPlaying())
        {
            this->menu_scene.music.Pause();
            this->pause_play_img.ref.set_image(this->menu_scene.assets.pause_png);
        }
        else
        {
            this->menu_scene.music.Seek(music_slider.ref.progress * this->menu_scene.music.GetTimeLength());
            this->menu_scene.music.Resume();
            this->pause_play_img.ref.set_image(this->menu_scene.assets.play_png);
        }
    };
}

menu::menu(kee::game& game, kee::global_assets& assets, beatmap_dir_info&& beatmap_info) :
    kee::scene::base(game, assets),
    k_text_alpha(add_transition<float>(0.0f)),
    k_rect(add_child<kee::ui::rect>(std::nullopt,
        kee::color::blank,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.25f)
        ),
        true,
        ui::rect_outline(ui::rect_outline::type::rel_h, 0.025f, kee::color(255, 255, 255, 0)),
        std::nullopt
    )),
    k_text(k_rect.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(255, 255, 255, k_text_alpha.get()),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.8f),
        std::nullopt, true, assets.font_semi_bold, "K", false
    )),
    e1_rect(add_child<kee::ui::rect>(std::nullopt,
        kee::color::blank,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.25f)
        ),
        true,
        ui::rect_outline(ui::rect_outline::type::rel_h, 0.025f, kee::color(255, 255, 255, 0)),
        std::nullopt
    )),
    e1_text(e1_rect.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(255, 255, 255, 0),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.8f),
        std::nullopt, true, assets.font_semi_bold, "E", false
    )),
    e2_rect(add_child<kee::ui::rect>(std::nullopt,
        kee::color::blank,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.25f)
        ),
        true,
        ui::rect_outline(ui::rect_outline::type::rel_h, 0.025f, kee::color(255, 255, 255, 0)),
        std::nullopt
    )),
    e2_text(e2_rect.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(255, 255, 255, 0),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.8f),
        std::nullopt, true, assets.font_semi_bold, "E", false
    )),
    music(std::move(beatmap_info.song)),
    music_time(0.f),
    scene_time(0.f)
{ 
    k_text_alpha.set(std::nullopt, 255.0f, 2.0f, kee::transition_type::lin);

    music.SetVolume(0.f);
}

void menu::update_element(float dt)
{
    music.Update();

    if (music_trns.has_value())
    {
        if (!music_trns.value().music_volume_trns_finished)
        {
            const float music_volume_multiplier = music_trns.value().music_volume_multiplier.get();
            music.SetVolume(0.01f * music_volume_multiplier);

            if (music_volume_multiplier >= 1.0f)
                music_trns.value().music_volume_trns_finished = true;
        }

        if (!music_trns.value().music_slider.ref.is_down())
        {
            if (music.IsPlaying())
                music_time = music.GetTimePlayed();
            
            music_trns.value().music_slider.ref.progress = music_time / music.GetTimeLength();
        }
        else
            music_time = music_trns.value().music_slider.ref.progress * music.GetTimeLength();

        music_trns.value().music_slider.ref.color.a = music_trns.value().slider_alpha.get();
        std::get<kee::dims>(music_trns.value().music_slider.ref.dimensions).w.val = music_trns.value().slider_width.get();

        auto& [w, h] = std::get<kee::dims>(music_trns.value().pause_play_img.ref.dimensions);
        w.val = music_trns.value().pause_play_scale.get();
        h.val = music_trns.value().pause_play_scale.get();
        music_trns.value().pause_play_img.ref.color = music_trns.value().pause_play_color.get();
    }

    scene_time += dt;
    if (scene_time >= 3.0f && !opening_trns.has_value())
        opening_trns.emplace(*this);

    if (scene_time >= 3.5f && !music_trns.has_value())
    {
        music_trns.emplace(*this);
        music.Play();
    }

    k_text.ref.color.a = k_text_alpha.get();
    k_rect.ref.outline.value().color.a = opening_trns.has_value() ? opening_trns.value().k_rect_alpha.get() : 0.f;
    k_rect.ref.x.val = opening_trns.has_value() ? opening_trns.value().k_rect_x.get() : 0.5f;

    e1_text.ref.color.a = opening_trns.has_value() ? opening_trns.value().e1_text_alpha.get() : 0.f;
    e1_rect.ref.outline.value().color.a = opening_trns.has_value() ? opening_trns.value().e1_rect_alpha.get() : 0.f;

    e2_text.ref.color.a = opening_trns.has_value() ? opening_trns.value().e2_text_alpha.get() : 0.f;
    e2_rect.ref.outline.value().color.a = opening_trns.has_value() ? opening_trns.value().e2_rect_alpha.get() : 0.f;
    e2_rect.ref.x.val = opening_trns.has_value() ? opening_trns.value().e2_rect_x.get() : 0.5f;
}

} // namespace scene
} // namespace kee