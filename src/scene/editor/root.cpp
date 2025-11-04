#include "kee/scene/editor/root.hpp"

namespace kee {
namespace scene {
namespace editor {

root::root(const kee::scene::window& window, kee::game& game, kee::global_assets& assets) :
    kee::scene::base(window, game, assets),
    exit_png("assets/img/exit.png"),
    error_png("assets/img/error.png"),
    pause_png("assets/img/pause.png"),
    arrow_png("assets/img/arrow.png"),
    active_tab_elem(add_child<timing_tab>(std::nullopt)),
    active_tab(root::tabs::timing),
    tab_active_rect_rel_x(add_transition<float>(static_cast<float>(active_tab) / magic_enum::enum_count<root::tabs>())),
    exit_button_rect_alpha(add_transition<float>(0.0f)),
    pause_play_color(add_transition<kee::color>(kee::color::white())),
    pause_play_scale(add_transition<float>(1.0f)),
    playback_l_img_color(add_transition<kee::color>(kee::color::white())),
    playback_l_img_scale(add_transition<float>(0.7f)),
    playback_r_img_color(add_transition<kee::color>(kee::color::white())),
    playback_r_img_scale(add_transition<float>(0.7f)),
    error_rect_rel_x(add_transition<float>(1.0f)),
    error_alpha(add_transition<float>(0)),
    tab_rect(add_child<kee::ui::rect>(1,
        raylib::Color(10, 10, 10, 255),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.04f)
        ),
        false, std::nullopt, std::nullopt
    )),
    tab_display_frame(tab_rect.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::abs, 0),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    tab_active_rect(tab_display_frame.ref.add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(20, 20, 20, 255),
        pos(pos::type::rel, tab_active_rect_rel_x.get()),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 1.0f / magic_enum::enum_count<root::tabs>()),
            dim(dim::type::rel, 1)
        ),
        false, std::nullopt, std::nullopt
    )),
    exit_button(tab_rect.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::end, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    exit_button_rect(exit_button.ref.add_child<kee::ui::rect>(0,
        raylib::Color(255, 0, 0, static_cast<unsigned char>(exit_button_rect_alpha.get())),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0),
        true, std::nullopt, std::nullopt
    )),
    exit_button_image(exit_button.ref.add_child<kee::ui::image>(1,
        exit_png,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_w, 0.3f),
        true, false, false, 0.0f
    )),
    playback_bg(add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(30, 30, 30),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.92f),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.08f)
        ),
        false, std::nullopt, std::nullopt
    )),
    playback_ui_frame(playback_bg.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.15f),
        true
    )),
    music_slider(playback_ui_frame.ref.add_child<kee::ui::slider>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.1f)
        ),
        false
    )),
    pause_play(playback_ui_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::beg, 0),
        pos(pos::type::end, 0),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.7f)
        ),
        false
    )),
    pause_play_img(pause_play.ref.add_child<kee::ui::image>(std::nullopt,
        assets.play_png,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        true, false, false, 0.0f
    )),
    music_time_text(playback_ui_frame.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::end, 0),
        pos(pos::type::end, 0),
        ui::text_size(ui::text_size::type::rel_h, 0.75f),
        false, assets.font_regular, std::string(), false
    )),
    playback_l_button(playback_ui_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.05f),
        pos(pos::type::rel, 0.3f),
        dims(
            dim(dim::type::rel, 0.02f),
            dim(dim::type::rel, 0.7f)
        ),
        false
    )),
    playback_l_bg(playback_l_button.ref.add_child<kee::ui::rect>(0,
        raylib::Color(20, 20, 20),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        border(border::type::rel_h, 0),
        false, std::nullopt, std::nullopt
    )),
    playback_l_img(playback_l_button.ref.add_child<kee::ui::image>(1,
        arrow_png, raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.15f),
        true, false, false, 0.0f
    )),
    playback_speed_idx(3),
    playback_text_bg(playback_ui_frame.ref.add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(10, 10, 10),
        pos(pos::type::rel, 0.07f),
        pos(pos::type::rel, 0.3f),
        dims(
            dim(dim::type::rel, 0.06f),
            dim(dim::type::rel, 0.7f)
        ),
        false, std::nullopt, std::nullopt
    )),
    playback_text(playback_text_bg.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.7f),
        true, assets.font_semi_bold, std::format("{:.2f}x", root::playback_speeds[playback_speed_idx]), false
    )),
    playback_r_button(playback_ui_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.13f),
        pos(pos::type::rel, 0.3f),
        dims(
            dim(dim::type::rel, 0.02f),
            dim(dim::type::rel, 0.7f)
        ),
        false
    )),
    playback_r_bg(playback_r_button.ref.add_child<kee::ui::rect>(0,
        raylib::Color(20, 20, 20),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        border(border::type::rel_h, 0),
        false, std::nullopt, std::nullopt
    )),
    playback_r_img(playback_r_button.ref.add_child<kee::ui::image>(std::nullopt,
        arrow_png, raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.15f),
        true, true, false, 0.0f
    )),
    error_rect(add_child<kee::ui::rect>(1,
        raylib::Color(80, 80, 80, static_cast<unsigned char>(error_alpha.get())),
        pos(pos::type::rel, error_rect_rel_x.get()),
        pos(pos::type::rel, 0.93f),
        dims(
            dim(dim::type::rel, 0.2f),
            dim(dim::type::rel, 0.05f)
        ),
        false,
        ui::rect_outline(ui::rect_outline::type::rel_h, 0.1f, raylib::Color(255, 0, 0, static_cast<unsigned char>(error_alpha.get()))),
        ui::rect_roundness(ui::rect_roundness::type::rel_h, 0.2f, std::nullopt)
    )),
    error_img_frame(error_rect.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    error_img(error_img_frame.ref.add_child<kee::ui::image>(std::nullopt,
        error_png,
        raylib::Color(255, 0, 0, static_cast<unsigned char>(error_alpha.get())),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.3f),
        true, false, false, 0.0f
    )),
    error_text(error_rect.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color(255, 255, 255, static_cast<unsigned char>(error_alpha.get())),
        pos(pos::type::rel, 0.13f),
        pos(pos::type::rel, 0.3f),
        ui::text_size(ui::text_size::type::rel_h, 0.4f),
        false, assets.font_semi_bold, std::string(), false
    )),
    music("assets/daft-punk-something-about-us/daft-punk-something-about-us.mp3"),
    music_time(0.0f),
    error_timer(0.0f),
    compose_info(music, arrow_png, music_time)
{
    std::visit([](const auto& elem) {
        elem.ref.take_keyboard_capture();
    }, active_tab_elem);

    const raylib::Rectangle tab_raw_rect = tab_rect.ref.get_raw_rect();
    std::get<kee::dims>(tab_display_frame.ref.dimensions).w.val = tab_raw_rect.width - tab_raw_rect.height;

    exit_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->exit_button_rect_alpha.set(std::nullopt, 255, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->exit_button_rect_alpha.set(std::nullopt, 0, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    tab_buttons.reserve(magic_enum::enum_count<root::tabs>());
    tab_button_text.reserve(magic_enum::enum_count<root::tabs>());

    for (std::size_t i = 0; i < magic_enum::enum_count<root::tabs>(); i++)
    {
        tab_button_text_colors.push_back(add_transition<kee::color>(kee::color::white()));

        tab_buttons.push_back(tab_display_frame.ref.add_child<kee::ui::button>(std::nullopt,
            pos(pos::type::rel, static_cast<float>(i) / magic_enum::enum_count<root::tabs>()),
            pos(pos::type::rel, 0),
            dims(
                dim(dim::type::rel, 1.0f / magic_enum::enum_count<root::tabs>()),
                dim(dim::type::rel, 1)
            ),
            false
        ));

        tab_buttons.back().ref.on_event = [&, idx = i](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
        {
            switch (button_event)
            {
            case ui::button::event::on_hot:
                this->tab_button_text_colors[idx].get().set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
                break;
            case ui::button::event::on_leave:
                this->tab_button_text_colors[idx].get().set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
                break;
            default:
                break;
            }
        };

        tab_buttons.back().ref.on_click_l = [&, idx = i]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
        {
            const float new_rel_x = static_cast<float>(idx) / magic_enum::enum_count<root::tabs>();
            this->tab_active_rect_rel_x.set(std::nullopt, new_rel_x, 0.3f, kee::transition_type::exp);

            const root::tabs tab_enum = static_cast<root::tabs>(idx);
            if (this->active_tab == tab_enum)
                return;

            std::visit([](const auto& elem) {
                elem.ref.release_keyboard_capture();
            }, this->active_tab_elem);

            this->active_tab = tab_enum;
            switch (this->active_tab)
            {
            case root::tabs::setup:
                this->active_tab_elem.emplace<kee::ui::handle<setup_tab>>(add_child<setup_tab>(std::nullopt, *this));
                break;
            case root::tabs::compose:
                this->active_tab_elem.emplace<kee::ui::handle<compose_tab>>(add_child<compose_tab>(std::nullopt, compose_info));
                break;
            case root::tabs::decoration:
                this->active_tab_elem.emplace<kee::ui::handle<decoration_tab>>(add_child<decoration_tab>(std::nullopt));
                break;
            case root::tabs::timing:
                this->active_tab_elem.emplace<kee::ui::handle<timing_tab>>(add_child<timing_tab>(std::nullopt));
                break;
            }

            std::visit([](const auto& elem) {
                elem.ref.take_keyboard_capture();
            }, this->active_tab_elem);
        };

        const root::tabs tab_enum = static_cast<root::tabs>(i);
        std::string enum_name = std::string(magic_enum::enum_name(tab_enum));
        std::transform(enum_name.begin(), enum_name.end(), enum_name.begin(), [](unsigned char c) { return static_cast<unsigned char>(std::toupper(c)); });

        tab_button_text.push_back(tab_buttons.back().ref.add_child<kee::ui::text>(std::nullopt,
            raylib::Color::White(),
            pos(pos::type::rel, 0.5f),
            pos(pos::type::rel, 0.5f),
            ui::text_size(ui::text_size::type::rel_h, 0.6f),
            true, assets.font_semi_bold, enum_name, false
        ));
    }

    music_slider.ref.on_event = [&, music_is_playing = music.IsPlaying()](ui::slider::event slider_event) mutable
    {
        switch (slider_event)
        {
        case ui::slider::event::on_down:
            music_is_playing = music.IsPlaying();
            this->music.Pause();
            break;
        case ui::slider::event::on_release:
            this->music.Seek(music_slider.ref.progress * this->music.GetTimeLength());
            if (music_is_playing)
                this->music.Resume();
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
            pause_play_color.set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
            pause_play_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            pause_play_scale.set(std::nullopt, 0.9f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            pause_play_color.set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
            pause_play_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    pause_play.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    { 
        if (this->music.IsPlaying())
        {
            this->music.Pause();
            this->pause_play_img.ref.set_image(this->assets.play_png);
        }
        else
        {
            this->music.Seek(music_slider.ref.progress * this->music.GetTimeLength());
            this->music.Resume();
            this->pause_play_img.ref.set_image(pause_png);            
        }
    };

    const unsigned int music_length = static_cast<unsigned int>(music.GetTimeLength());
    const std::string music_length_str = std::format("0:00 / {}:{:02}", music_length / 60, music_length % 60);
    music_time_text.ref.set_string(music_length_str);

    playback_l_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            playback_l_img_color.set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
            playback_l_img_scale.set(std::nullopt, 0.7f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            playback_l_img_scale.set(std::nullopt, 0.6f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            playback_l_img_color.set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
            playback_l_img_scale.set(std::nullopt, 0.7f, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    playback_l_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (this->playback_speed_idx > 0)
        {
            this->playback_speed_idx--;
            this->playback_text.ref.set_string(std::format("{:.2f}x", root::playback_speeds[this->playback_speed_idx]));
            this->music.SetPitch(root::playback_speeds[this->playback_speed_idx]);
        }
    };

    playback_r_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (this->playback_speed_idx < root::playback_speeds.size() - 1)
        {
            this->playback_speed_idx++;
            this->playback_text.ref.set_string(std::format("{:.2f}x", root::playback_speeds[this->playback_speed_idx]));
            this->music.SetPitch(root::playback_speeds[this->playback_speed_idx]);
        }
    };

    playback_r_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            playback_r_img_color.set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
            playback_r_img_scale.set(std::nullopt, 0.7f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            playback_r_img_scale.set(std::nullopt, 0.6f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            playback_r_img_color.set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
            playback_r_img_scale.set(std::nullopt, 0.7f, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    music.SetLooping(true);
    music.SetVolume(0.1f);
    /**
     * First `.Play()` on a `raylib::Music` is slow.
     */
    music.Play();
    music.Pause();
}

void root::set_error(std::string_view error_str, bool from_file_dialog)
{
    error_text.ref.set_string(error_str);
    /**
     * File dialogs hang the program during file selection, making that frame's 
     * `dt` significantly large, messing up transition timings for the error's UI.
     * These frames are skipped before error UI rendering is triggered.
     */
    error_skips_before_start = from_file_dialog ? 2 : 0;
}

bool root::on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods)
{
    if (keycode != KeyboardKey::KEY_SPACE || mods.test(kee::mods::ctrl))
        return false;

    pause_play.ref.on_click_l(mods);
    return true;
}

void root::update_element(float dt)
{
    music.Update();

    if (!music_slider.ref.is_down())
    {
        if (music.IsPlaying())
            music_time = music.GetTimePlayed();

        music_slider.ref.progress = music_time / music.GetTimeLength();
    }
    else
        music_time = music_slider.ref.progress * music.GetTimeLength();

    std::get<kee::dims>(pause_play_img.ref.dimensions).w.val = pause_play_scale.get();
    std::get<kee::dims>(pause_play_img.ref.dimensions).h.val = pause_play_scale.get();
    pause_play_img.ref.set_opt_color(pause_play_color.get().to_color());

    const unsigned int music_length = static_cast<unsigned int>(compose_info.music.GetTimeLength());
    const unsigned int music_time_int = static_cast<unsigned int>(compose_info.music_time);
    const std::string music_time_str = std::format("{}:{:02} / {}:{:02}", music_time_int / 60, music_time_int % 60, music_length / 60, music_length % 60);
    music_time_text.ref.set_string(music_time_str);

    if (error_timer > 0.0f)
    {
        error_timer -= dt;
        if (error_timer <= 0.0f)
        {
            error_alpha.set(255.0f, 0.0f, root::error_transition_time, kee::transition_type::exp);
            error_rect_rel_x.set(0.79f, 1.0f, root::error_transition_time, kee::transition_type::exp);

            error_timer = 0.0f;
        }
    }

    if (error_skips_before_start.has_value())
    {
        if (error_skips_before_start.value() != 0)
            error_skips_before_start.value()--;
        else
        {
            error_alpha.set(0.0f, 255.0f, root::error_transition_time, kee::transition_type::exp);
            error_rect_rel_x.set(1.0f, 0.79f, root::error_transition_time, kee::transition_type::exp);

            error_timer = 3.0f + root::error_transition_time;
            error_skips_before_start.reset();
        }
    }

    for (std::size_t i = 0; i < tab_button_text.size(); i++)
        tab_button_text[i].ref.set_opt_color(tab_button_text_colors[i].get().get().to_color());

    tab_active_rect.ref.x.val = tab_active_rect_rel_x.get();
    exit_button_rect.ref.set_opt_color(raylib::Color(255, 0, 0, static_cast<unsigned char>(exit_button_rect_alpha.get())));

    const float playback_l_scale = (playback_speed_idx > 0) ? playback_l_img_scale.get() : 0.7f;
    const float playback_r_scale = (playback_speed_idx < root::playback_speeds.size() - 1) ? playback_r_img_scale.get() : 0.7f;
    std::get<kee::border>(playback_l_img.ref.dimensions).val = (1.0f - playback_l_scale) / 2;
    std::get<kee::border>(playback_r_img.ref.dimensions).val = (1.0f - playback_r_scale) / 2;

    const raylib::Color playback_l_color = (playback_speed_idx > 0) ? playback_l_img_color.get().to_color() : raylib::Color(255, 255, 255, 80);
    const raylib::Color playback_r_color = (playback_speed_idx < root::playback_speeds.size() - 1) ? playback_r_img_color.get().to_color() : raylib::Color(255, 255, 255, 80);
    playback_l_img.ref.set_opt_color(playback_l_color);
    playback_r_img.ref.set_opt_color(playback_r_color);

    const unsigned char error_a = static_cast<unsigned char>(error_alpha.get());
    error_rect.ref.set_opt_color(raylib::Color(80, 80, 80, error_a));
    error_rect.ref.border.value().opt_color = raylib::Color(255, 0, 0, error_a);
    error_img.ref.set_opt_color(raylib::Color(255, 0, 0, error_a));
    error_text.ref.set_opt_color(raylib::Color(255, 255, 255, error_a));
    error_rect.ref.x.val = error_rect_rel_x.get();
}

} // namespace editor
} // namespace scene
} // namespace kee