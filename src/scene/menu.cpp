#include "kee/scene/menu.hpp"

namespace kee {
namespace scene {

music_analyzer::music_analyzer(const std::filesystem::path& music_path) :
    wave(music_path.string()),
    frame_cursor(0),
    audio_stream(music_analyzer::sample_rate, 8 * sizeof(sample_t))
{ 
    audio_stream.SetBufferSizeDefault(music_analyzer::samples_per_refresh);
}

void music_analyzer::update()
{
    while (audio_stream.IsProcessed())
    {

    }
}

void music_analyzer::play()
{
    audio_stream.Play();
}

void music_analyzer::pause()
{
    audio_stream.Pause();
}

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
    step_l_color(menu_scene.add_transition<kee::color>(kee::color(255, 255, 255, 0))),
    step_r_color(menu_scene.add_transition<kee::color>(kee::color(255, 255, 255, 0))),
    setting_color(menu_scene.add_transition<kee::color>(kee::color(255, 255, 255, 0))),
    exit_color(menu_scene.add_transition<kee::color>(kee::color(255, 255, 255, 0))),
    pause_play_border(menu_scene.add_transition<float>(0.f)),
    step_l_border(menu_scene.add_transition<float>(0.f)),
    step_r_border(menu_scene.add_transition<float>(0.f)),
    setting_border(menu_scene.add_transition<float>(0.f)),
    exit_border(menu_scene.add_transition<float>(0.f)),
    song_ui_alpha(menu_scene.add_transition<float>(0.f)),
    step_texture("assets/img/step.png"),
    setting_texture("assets/img/settings.png"),
    music_slider(menu_scene.add_child<kee::ui::slider>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.895f),
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
        border(border::type::rel_h, pause_play_border.get()),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    step_l(menu_scene.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.4f),
        pos(pos::type::rel, 0.95f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.04f)
        ),
        true
    )),
    step_l_img(step_l.ref.add_child<kee::ui::image>(std::nullopt,
        step_texture, step_l_color.get(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, step_l_border.get()),
        true, ui::image::display::shrink_to_fit, true, false, 0.0f
    )),
    step_r(menu_scene.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.6f),
        pos(pos::type::rel, 0.95f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.04f)
        ),
        true
    )),
    step_r_img(step_r.ref.add_child<kee::ui::image>(std::nullopt,
        step_texture, step_r_color.get(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, step_r_border.get()),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    setting_exit_frame(menu_scene.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_w, 0.02f),
        true
    )),
    setting_button(setting_exit_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::beg, 0.f),
        pos(pos::type::beg, 0.f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.04f)
        ),
        false
    )),
    setting_img(setting_button.ref.add_child<kee::ui::image>(std::nullopt,
        setting_texture, setting_color.get(),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        border(border::type::rel_h, setting_border.get()),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    exit_button(setting_exit_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::end, 0.f),
        pos(pos::type::beg, 0.f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.04f)
        ),
        false
    )),
    exit_img(exit_button.ref.add_child<kee::ui::image>(std::nullopt,
        menu_scene.assets.exit_png, exit_color.get(),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        border(border::type::rel_h, exit_border.get()),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    ))
{
    slider_alpha.set(std::nullopt, 255.0f, 0.5f, kee::transition_type::lin);
    slider_width.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
    music_volume_multiplier.set(std::nullopt, 2.0f, 2.0f, kee::transition_type::inv_exp);
    pause_play_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::lin);
    step_l_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::lin);
    step_r_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::lin);
    setting_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::lin);
    exit_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::lin);
    song_ui_alpha.set(std::nullopt, 255.f, 0.5f, kee::transition_type::lin);

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
            pause_play_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            pause_play_border.set(std::nullopt, 0.05f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            pause_play_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
            pause_play_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
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

    step_l.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            step_l_color.set(std::nullopt, kee::color(255, 255, 255, 200), 0.5f, kee::transition_type::exp);
            step_l_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            step_l_border.set(std::nullopt, 0.05f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            step_l_color.set(std::nullopt, kee::color(255, 255, 255), 0.5f, kee::transition_type::exp);
            step_l_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    step_l.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    { 
        /* TODO: impl*/
    };

    step_r.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            step_r_color.set(std::nullopt, kee::color(255, 255, 255, 200), 0.5f, kee::transition_type::exp);
            step_r_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            step_r_border.set(std::nullopt, 0.05f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            step_r_color.set(std::nullopt, kee::color(255, 255, 255), 0.5f, kee::transition_type::exp);
            step_r_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    step_r.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    { 
        /* TODO: impl*/
    };

    setting_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            setting_color.set(std::nullopt, kee::color(255, 255, 255, 200), 0.5f, kee::transition_type::exp);
            setting_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            setting_border.set(std::nullopt, 0.05f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            setting_color.set(std::nullopt, kee::color(255, 255, 255), 0.5f, kee::transition_type::exp);
            setting_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    setting_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    { 
        /* TODO: impl*/
    };

    exit_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            exit_color.set(std::nullopt, kee::color(255, 255, 255, 200), 0.5f, kee::transition_type::exp);
            exit_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            exit_border.set(std::nullopt, 0.05f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            exit_color.set(std::nullopt, kee::color(255, 255, 255), 0.5f, kee::transition_type::exp);
            exit_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    setting_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    { 
        /* TODO: impl*/
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
    music_cover_art_texture(beatmap_info.dir_state.has_image
        ? std::make_optional(beatmap_info.dir_state.path / beatmap_dir_state::standard_img_filename)
        : std::nullopt
    ),
    music(std::move(beatmap_info.song)),
    music_time(0.f),
    song_ui_frame_outer(add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 0.9f),
        dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::rel, 0.1f)
        ),
        false
    )),
    song_ui_frame_inner(song_ui_frame_outer.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.25f),
        true
    )),
    music_cover_art_frame(song_ui_frame_inner.ref.add_child<kee::ui::rect>(std::nullopt,
        kee::color(255, 255, 255, 0),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::aspect, static_cast<float>(kee::window_w) / kee::window_h),
            dim(dim::type::rel, 1)
        ),
        false, std::nullopt, std::nullopt
    )),
    music_cover_art(music_cover_art_frame.ref.add_child<kee::ui::image>(std::nullopt,
        /* TODO: if cover art texture doesn't exist handle */
        music_cover_art_texture.value(), 
        kee::color(255, 255, 255, 0),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    music_info_text_frame(song_ui_frame_outer.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.25f),
        dims(
            dim(dim::type::rel, 0.01f),
            dim(dim::type::rel, 0.5f)
        ),
        false
    )),
    music_name_text(music_info_text_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(255, 255, 255, 0),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        ui::text_size(ui::text_size::type::rel_h, 0.6f),
        std::nullopt, false, assets.font_semi_bold, beatmap_info.song_name, false
    )),
    music_artist_text(music_info_text_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(255, 255, 255, 0),
        pos(pos::type::beg, 0),
        pos(pos::type::end, 0),
        ui::text_size(ui::text_size::type::rel_h, 0.35f),
        std::nullopt, false, assets.font_regular, beatmap_info.song_artist, true
    )),
    music_time_text(song_ui_frame_inner.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(255, 255, 255, 0),
        pos(pos::type::end, 0),
        pos(pos::type::beg, 0),
        ui::text_size(ui::text_size::type::rel_h, 1.0f),
        std::nullopt, false, assets.font_regular, std::string(), false
    )),
    scene_time(0.f)
{ 
    const raylib::Rectangle music_cover_art_frame_rect = music_cover_art_frame.ref.get_raw_rect();
    const float music_cover_art_frame_rel_beg = music_cover_art_frame_rect.x / kee::window_w;
    const float music_cover_art_frame_rel_end = (music_cover_art_frame_rect.x + music_cover_art_frame_rect.width)/ kee::window_w;
    music_info_text_frame.ref.x.val = music_cover_art_frame_rel_end + music_cover_art_frame_rel_beg / 2.f;

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

        std::get<kee::border>(music_trns.value().pause_play_img.ref.dimensions).val = music_trns.value().pause_play_border.get();
        std::get<kee::border>(music_trns.value().step_l_img.ref.dimensions).val = music_trns.value().step_l_border.get();
        std::get<kee::border>(music_trns.value().step_r_img.ref.dimensions).val = music_trns.value().step_r_border.get();
        std::get<kee::border>(music_trns.value().setting_img.ref.dimensions).val = music_trns.value().setting_border.get();
        std::get<kee::border>(music_trns.value().exit_img.ref.dimensions).val = music_trns.value().exit_border.get();

        music_trns.value().pause_play_img.ref.color = music_trns.value().pause_play_color.get();
        music_trns.value().step_l_img.ref.color = music_trns.value().step_l_color.get();
        music_trns.value().step_r_img.ref.color = music_trns.value().step_r_color.get();
        music_trns.value().setting_img.ref.color = music_trns.value().setting_color.get();
        music_trns.value().exit_img.ref.color = music_trns.value().exit_color.get();

        const unsigned int music_length = static_cast<unsigned int>(music.GetTimeLength());
        const unsigned int music_time_int = static_cast<unsigned int>(music_time);
        const std::string music_time_str = std::format("{}:{:02} / {}:{:02}", music_time_int / 60, music_time_int % 60, music_length / 60, music_length % 60);
        music_time_text.ref.set_string(music_time_str);

        music_cover_art_frame.ref.color.a = (20.f * music_trns.value().song_ui_alpha.get()) / 255.f;
        music_time_text.ref.color.a = music_trns.value().song_ui_alpha.get();
        music_name_text.ref.color.a = music_trns.value().song_ui_alpha.get();
        music_artist_text.ref.color.a = (100.f * music_trns.value().song_ui_alpha.get()) / 255.f;
        music_cover_art.ref.color.a = music_trns.value().song_ui_alpha.get();
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