#include "kee/scene/editor/root.hpp"

#include "kee/game.hpp"

namespace kee {
namespace scene {
namespace editor {

beatmap_file::beatmap_file(const std::filesystem::path& file_dir) :
    file_dir(file_dir),
    save_metadata_needed(false)
{ }

confirm_exit_ui::confirm_exit_ui(const kee::ui::required& reqs, float menu_width) :
    kee::ui::base(reqs,
        pos(pos::type::end, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::abs, 0),
            dim(dim::type::rel, 1)
        ),
        false
    ),
    menu_width(menu_width),
    base_w(add_transition<float>(0)),
    confirm_button_text_color(add_transition<kee::color>(kee::color::white)),
    go_back_button_text_color(add_transition<kee::color>(kee::color::white)),
    confirm_button(add_child<kee::ui::button>(-1,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 0.5f),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    confirm_button_bg(confirm_button.ref.add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(200, 0, 0),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        border(border::type::abs, 0),
        true, std::nullopt, std::nullopt
    )),
    confirm_button_text(confirm_button_bg.ref.make_temp_child<kee::ui::text>(
        confirm_button_text_color.get().to_color(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.75f),
        true, assets.font_semi_bold, "CONFIRM", false
    )),
    go_back_button(add_child<kee::ui::button>(-1,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 0.5f),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    go_back_button_bg(go_back_button.ref.add_child<kee::ui::rect>(std::nullopt,
        raylib::Color::DarkGreen(),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        border(border::type::abs, 0),
        true, std::nullopt, std::nullopt
    )),
    go_back_button_text(go_back_button_bg.ref.make_temp_child<kee::ui::text>(
        go_back_button_text_color.get().to_color(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.75f),
        true, assets.font_semi_bold, "GO BACK", false
    ))
{
    take_render_priority();
    base_w.set(get_raw_rect().height, menu_width, confirm_exit_ui::transition_time, kee::transition_type::exp);

    confirm_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->confirm_button_text_color.set(std::nullopt, kee::color::dark_orange, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->confirm_button_text_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    confirm_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (!this->destroy_timer.has_value())
            this->game_ref.queue_game_exit();
    };

    go_back_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->go_back_button_text_color.set(std::nullopt, kee::color::dark_orange, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->go_back_button_text_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    go_back_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (!this->destroy_timer.has_value())
            this->queue_for_destruction();
    };
}

bool confirm_exit_ui::should_destruct() const
{
    return destroy_timer.has_value() && destroy_timer.value() <= 0;
}

bool confirm_exit_ui::on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    if (is_mouse_l && !destroy_timer.has_value() && !get_raw_rect().CheckCollision(mouse_pos))
        queue_for_destruction();

    return false;
}

void confirm_exit_ui::update_element(float dt)
{
    if (destroy_timer.has_value())
        destroy_timer.value() -= dt;

    std::get<kee::dims>(dimensions).w.val = base_w.get();
    confirm_button_text.set_opt_color(confirm_button_text_color.get().to_color());
    go_back_button_text.set_opt_color(go_back_button_text_color.get().to_color());
}

void confirm_exit_ui::render_element() const
{
    const raylib::Rectangle confirm_button_rect = confirm_button.ref.get_raw_rect();
    const raylib::Rectangle go_back_button_rect = go_back_button.ref.get_raw_rect();

    BeginScissorMode(
        static_cast<int>(confirm_button_rect.x),
        static_cast<int>(confirm_button_rect.y),
        static_cast<int>(confirm_button_rect.width),
        static_cast<int>(confirm_button_rect.height)
    );
    confirm_button_text.render();
    EndScissorMode();

    BeginScissorMode(
        static_cast<int>(go_back_button_rect.x),
        static_cast<int>(go_back_button_rect.y),
        static_cast<int>(go_back_button_rect.width),
        static_cast<int>(go_back_button_rect.height)
    );
    go_back_button_text.render();
    EndScissorMode();
}

void confirm_exit_ui::queue_for_destruction()
{
    if (destroy_timer.has_value())
        return;

    release_render_priority();
    destroy_timer = confirm_exit_ui::transition_time;
    base_w.set(std::nullopt, 0, confirm_exit_ui::transition_time, kee::transition_type::exp);
}

song_ui::song_ui(
    const kee::ui::required& reqs, 
    const kee::image_texture& arrow_png,
    const std::filesystem::path& music_path
) :
    kee::ui::base(reqs,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0),
        true
    ),
    music_bpm(60.0f),
    music_start_offset(0),
    arrow_png(arrow_png),
    pause_play_color(add_transition<kee::color>(kee::color::white)),
    pause_play_scale(add_transition<float>(1.0f)),
    playback_l_img_color(add_transition<kee::color>(kee::color::white)),
    playback_l_img_scale(add_transition<float>(0.7f)),
    playback_r_img_color(add_transition<kee::color>(kee::color::white)),
    playback_r_img_scale(add_transition<float>(0.7f)),
    music_slider(add_child<kee::ui::slider>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.1f)
        ),
        false
    )),
    pause_play(add_child<kee::ui::button>(std::nullopt,
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
    music_time_text(add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::end, 0),
        pos(pos::type::end, 0),
        ui::text_size(ui::text_size::type::rel_h, 0.75f),
        false, assets.font_regular, std::string(), false
    )),
    playback_l_button(add_child<kee::ui::button>(std::nullopt,
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
    playback_text_bg(add_child<kee::ui::rect>(std::nullopt,
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
        true, assets.font_semi_bold, std::format("{:.2f}x", song_ui::playback_speeds[playback_speed_idx]), false
    )),
    playback_r_button(add_child<kee::ui::button>(std::nullopt,
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
    music(music_path.string()),
    music_time(0.0f)
{
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
            pause_play_color.set(std::nullopt, kee::color::dark_orange, 0.5f, kee::transition_type::exp);
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
        if (this->music.IsPlaying())
        {
            this->music.Pause();
            this->pause_play_img.ref.set_image(this->assets.play_png);
        }
        else
        {
            this->music.Seek(music_slider.ref.progress * this->music.GetTimeLength());
            this->music.Resume();
            this->pause_play_img.ref.set_image(this->assets.pause_png);
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
            playback_l_img_color.set(std::nullopt, kee::color::dark_orange, 0.5f, kee::transition_type::exp);
            playback_l_img_scale.set(std::nullopt, 0.7f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            playback_l_img_scale.set(std::nullopt, 0.6f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            playback_l_img_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
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
            this->playback_text.ref.set_string(std::format("{:.2f}x", song_ui::playback_speeds[this->playback_speed_idx]));
            this->music.SetPitch(song_ui::playback_speeds[this->playback_speed_idx]);
        }
    };

    playback_r_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (this->playback_speed_idx < song_ui::playback_speeds.size() - 1)
        {
            this->playback_speed_idx++;
            this->playback_text.ref.set_string(std::format("{:.2f}x", song_ui::playback_speeds[this->playback_speed_idx]));
            this->music.SetPitch(song_ui::playback_speeds[this->playback_speed_idx]);
        }
    };

    playback_r_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            playback_r_img_color.set(std::nullopt, kee::color::dark_orange, 0.5f, kee::transition_type::exp);
            playback_r_img_scale.set(std::nullopt, 0.7f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            playback_r_img_scale.set(std::nullopt, 0.6f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            playback_r_img_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
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

const raylib::Music& song_ui::get_music() const
{
    return music;
}

const std::optional<float>& song_ui::get_prev_beat() const
{
    return prev_beat;
}

float song_ui::get_beat() const
{
    return (music_time - music_start_offset) * music_bpm / 60.0f;
}

void song_ui::set_beat(float new_beat)
{
    const float music_time_raw = music_start_offset + new_beat * 60.0f / music_bpm;
    music_time = std::clamp(music_time_raw, 0.0f, music.GetTimeLength());
}

void song_ui::pause_play_click(magic_enum::containers::bitset<kee::mods> mods)
{
    pause_play.ref.on_click_l(mods);
}

void song_ui::update_element([[maybe_unused]] float dt)
{
    music.Update();

    prev_beat.reset();
    if (!music_slider.ref.is_down())
    {
        if (music.IsPlaying())
        {
            music_time = music.GetTimePlayed();
            if (last_frame_beat.has_value() && last_frame_beat.value() < get_beat())
                prev_beat = last_frame_beat.value();

            last_frame_beat = get_beat();
        }

        music_slider.ref.progress = music_time / music.GetTimeLength();
    }
    else
        music_time = music_slider.ref.progress * music.GetTimeLength();

    std::get<kee::dims>(pause_play_img.ref.dimensions).w.val = pause_play_scale.get();
    std::get<kee::dims>(pause_play_img.ref.dimensions).h.val = pause_play_scale.get();
    pause_play_img.ref.set_opt_color(pause_play_color.get().to_color());

    const unsigned int music_length = static_cast<unsigned int>(music.GetTimeLength());
    const unsigned int music_time_int = static_cast<unsigned int>(music_time);
    const std::string music_time_str = std::format("{}:{:02} / {}:{:02}", music_time_int / 60, music_time_int % 60, music_length / 60, music_length % 60);
    music_time_text.ref.set_string(music_time_str);

    const float playback_l_scale = (playback_speed_idx > 0) ? playback_l_img_scale.get() : 0.7f;
    const float playback_r_scale = (playback_speed_idx < song_ui::playback_speeds.size() - 1) ? playback_r_img_scale.get() : 0.7f;
    std::get<kee::border>(playback_l_img.ref.dimensions).val = (1.0f - playback_l_scale) / 2;
    std::get<kee::border>(playback_r_img.ref.dimensions).val = (1.0f - playback_r_scale) / 2;

    const raylib::Color playback_l_color = (playback_speed_idx > 0) ? playback_l_img_color.get().to_color() : raylib::Color(255, 255, 255, 80);
    const raylib::Color playback_r_color = (playback_speed_idx < song_ui::playback_speeds.size() - 1) ? playback_r_img_color.get().to_color() : raylib::Color(255, 255, 255, 80);
    playback_l_img.ref.set_opt_color(playback_l_color);
    playback_r_img.ref.set_opt_color(playback_r_color);
}

root::root(const kee::scene::window& window, kee::game& game, kee::global_assets& assets) :
    kee::scene::base(window, game, assets),
    arrow_png("assets/img/arrow.png"),
    error_png("assets/img/error.png"),
    exit_png("assets/img/exit.png"),
    info_png("assets/img/info.png"),
    approach_beats(2.0f),
    compose_info(arrow_png),
    active_tab_elem(add_child<setup_tab>(std::nullopt, *this, setup_info, approach_beats)),
    active_tab(root::tabs::setup),
    tab_active_rect_rel_x(add_transition<float>(static_cast<float>(active_tab) / magic_enum::enum_count<root::tabs>())),
    exit_button_rect_alpha(add_transition<float>(0.0f)),
    notif_rect_rel_x(add_transition<float>(1.0f)),
    notif_alpha(add_transition<float>(0)),
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
    playback_ui(playback_ui_frame.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        ui::text_size(ui::text_size::type::rel_h, 0.5f),
        false, assets.font_semi_bold, "NO SONG SELECTED", false
    )),
    notif_rect(add_child<kee::ui::rect>(1,
        raylib::Color(80, 80, 80, static_cast<unsigned char>(notif_alpha.get())),
        pos(pos::type::rel, notif_rect_rel_x.get()),
        pos(pos::type::rel, 0.93f),
        dims(
            dim(dim::type::rel, 0.2f),
            dim(dim::type::rel, 0.05f)
        ),
        false,
        ui::rect_outline(ui::rect_outline::type::rel_h, 0.1f, raylib::Color(255, 0, 0, static_cast<unsigned char>(notif_alpha.get()))),
        ui::rect_roundness(ui::rect_roundness::type::rel_h, 0.2f, std::nullopt)
    )),
    notif_img_frame(notif_rect.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    notif_img(notif_img_frame.ref.add_child<kee::ui::image>(std::nullopt,
        error_png,
        raylib::Color(255, 0, 0, static_cast<unsigned char>(notif_alpha.get())),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.3f),
        true, false, false, 0.0f
    )),
    notif_text(notif_rect.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color(255, 255, 255, static_cast<unsigned char>(notif_alpha.get())),
        pos(pos::type::rel, 0.13f),
        pos(pos::type::rel, 0.3f),
        ui::text_size(ui::text_size::type::rel_h, 0.4f),
        false, assets.font_semi_bold, std::string(), false
    )),
    notif_timer(0.0f)
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

    exit_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        this->confirm_exit.emplace(this->exit_button.ref.add_child<confirm_exit_ui>(2, this->tab_rect.ref.get_raw_rect().width));
    };

    tab_buttons.reserve(magic_enum::enum_count<root::tabs>());
    tab_button_text.reserve(magic_enum::enum_count<root::tabs>());

    for (std::size_t i = 0; i < magic_enum::enum_count<root::tabs>(); i++)
    {
        tab_button_text_colors.push_back(add_transition<kee::color>(kee::color::white));

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
                this->tab_button_text_colors[idx].get().set(std::nullopt, kee::color::dark_orange, 0.5f, kee::transition_type::exp);
                break;
            case ui::button::event::on_leave:
                this->tab_button_text_colors[idx].get().set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
                break;
            default:
                break;
            }
        };

        tab_buttons.back().ref.on_click_l = [&, idx = i]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
        {
            const root::tabs tab_enum = static_cast<root::tabs>(idx);
            if (this->active_tab == tab_enum)
                return;

            const bool requires_song_ui = (tab_enum == root::tabs::compose || tab_enum == root::tabs::timing);
            if (requires_song_ui && !std::holds_alternative<kee::ui::handle<song_ui>>(this->playback_ui))
            {
                this->set_error("Select a song first!", false);
                return;
            }

            const float new_rel_x = static_cast<float>(idx) / magic_enum::enum_count<root::tabs>();
            this->tab_active_rect_rel_x.set(std::nullopt, new_rel_x, 0.3f, kee::transition_type::exp);
            this->active_tab = tab_enum;

            std::visit([](const auto& elem) {
                elem.ref.release_keyboard_capture();
            }, this->active_tab_elem);

            switch (tab_enum)
            {
            case root::tabs::setup:
                this->active_tab_elem.emplace<kee::ui::handle<setup_tab>>(add_child<setup_tab>(std::nullopt, *this, setup_info, approach_beats));
                break;
            case root::tabs::compose: {
                song_ui& song_ui_elem = std::get<kee::ui::handle<song_ui>>(this->playback_ui).ref;
                this->active_tab_elem.emplace<kee::ui::handle<compose_tab>>(add_child<compose_tab>(std::nullopt, approach_beats, song_ui_elem, compose_info));
                break;
            }
            case root::tabs::decoration:
                this->active_tab_elem.emplace<kee::ui::handle<decoration_tab>>(add_child<decoration_tab>(std::nullopt));
                break;
            case root::tabs::timing: {
                song_ui& song_ui_elem = std::get<kee::ui::handle<song_ui>>(this->playback_ui).ref;
                this->active_tab_elem.emplace<kee::ui::handle<timing_tab>>(add_child<timing_tab>(std::nullopt, *this, song_ui_elem, timing_info));
                break;
            }}

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
}

void root::save_beatmap()
{
    if (!save_info.has_value())
        throw std::runtime_error("Saving beatmap when no directory for it exists.");

    bool any_saved = false;
    if (setup_info.new_song_path.has_value())
    {
        std::error_code ec;
        const std::filesystem::path& src = setup_info.new_song_path.value();
        const std::filesystem::path dst = save_info.value().file_dir / "song.mp3";
        
        std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec);
        if (ec)
        {
            set_error(std::format("Error saving song: {}", ec.message()), false);
            return;
        }

        setup_info.new_song_path.reset();
        any_saved = true;
    }

    if (save_info.value().save_metadata_needed)
    {
        std::println("NEED TO SAVE METADATA");
        any_saved = true;
    }

    if (any_saved)
        set_info("Beatmap saved");
}

void root::set_error(std::string_view error_str, bool from_file_dialog)
{
    const raylib::Color old_notif_color = notif_img.ref.get_opt_color().value();
    const raylib::Color new_notif_color = raylib::Color(255, 0, 0, old_notif_color.a);

    notif_img.ref.set_image(error_png);
    notif_img.ref.set_opt_color(new_notif_color);
    notif_rect.ref.border.value().opt_color = new_notif_color;
    notif_text.ref.set_string(error_str);
    /**
     * File dialogs hang the program during file selection, making that frame's 
     * `dt` significantly large, messing up transition timings for the error's UI.
     * These frames are skipped before error UI rendering is triggered.
     */
    notif_skips_before_start = from_file_dialog ? 2 : 0;
}

void root::set_info(std::string_view info_str)
{
    const raylib::Color old_notif_color = notif_img.ref.get_opt_color().value();
    const raylib::Color new_notif_color = raylib::Color(144, 213, 255, old_notif_color.a);

    notif_img.ref.set_image(info_png);
    notif_img.ref.set_opt_color(new_notif_color);
    notif_rect.ref.border.value().opt_color = new_notif_color;
    notif_text.ref.set_string(info_str);

    notif_skips_before_start = 0;
}


void root::set_song(const std::filesystem::path& song_path)
{
    playback_ui.emplace<kee::ui::handle<song_ui>>(playback_ui_frame.ref.add_child<song_ui>(std::nullopt, arrow_png, song_path));
}

const std::filesystem::path root::app_data_dir = "test_app_data/";

bool root::on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods)
{
    switch (keycode)
    {
    case KeyboardKey::KEY_SPACE:
        if (mods.test(kee::mods::ctrl) || !std::holds_alternative<kee::ui::handle<song_ui>>(playback_ui))
            return false;

        std::get<kee::ui::handle<song_ui>>(playback_ui).ref.pause_play_click(mods);
        return true;
    case KeyboardKey::KEY_S: {
        if (!mods.test(kee::mods::ctrl))
            return false;

        if (this->save_info.has_value())
        {
            this->save_beatmap();
            return true;
        }

        if (!std::holds_alternative<kee::ui::handle<song_ui>>(this->playback_ui))
        {
            this->set_error("Choose a song before saving!", false);
            return true;
        }

        std::error_code ec;
        const std::filesystem::path new_path = root::app_data_dir / "local_0";
        const bool success = std::filesystem::create_directory(new_path, ec);

        if (ec)
        {
            this->set_error(std::format("Error saving: {}", ec.message()), false);
            return true;
        }

        if (!success)
            throw std::runtime_error("Directory already exists");

        this->save_info = beatmap_file(new_path);
        this->save_beatmap();
        return true;
    }
    default:
        return false;
    }
}

void root::update_element(float dt)
{
    if (notif_timer > 0.0f)
    {
        notif_timer -= dt;
        if (notif_timer <= 0.0f)
        {
            notif_alpha.set(255.0f, 0.0f, root::error_transition_time, kee::transition_type::exp);
            notif_rect_rel_x.set(0.79f, 1.0f, root::error_transition_time, kee::transition_type::exp);

            notif_timer = 0.0f;
        }
    }

    if (notif_skips_before_start.has_value())
    {
        if (notif_skips_before_start.value() != 0)
            notif_skips_before_start.value()--;
        else
        {
            notif_alpha.set(0.0f, 255.0f, root::error_transition_time, kee::transition_type::exp);
            notif_rect_rel_x.set(1.0f, 0.79f, root::error_transition_time, kee::transition_type::exp);

            notif_timer = 2.5f;
            notif_skips_before_start.reset();
        }
    }

    for (std::size_t i = 0; i < tab_button_text.size(); i++)
        tab_button_text[i].ref.set_opt_color(tab_button_text_colors[i].get().get().to_color());

    raylib::Color exit_button_rect_color = exit_button_rect.ref.get_opt_color().value();
    exit_button_rect_color.a = static_cast<unsigned char>(exit_button_rect_alpha.get());
    exit_button_rect.ref.set_opt_color(exit_button_rect_color);
    tab_active_rect.ref.x.val = tab_active_rect_rel_x.get();

    if (confirm_exit.has_value() && confirm_exit.value().ref.should_destruct())
        confirm_exit.reset();

    const unsigned char error_a = static_cast<unsigned char>(notif_alpha.get());
    const raylib::Color notif_rect_border_color = notif_rect.ref.border.value().opt_color.value();
    const raylib::Color notif_img_color = notif_img.ref.get_opt_color().value();
    
    notif_rect.ref.set_opt_color(raylib::Color(80, 80, 80, error_a));
    notif_rect.ref.border.value().opt_color = raylib::Color(notif_rect_border_color.r, notif_rect_border_color.g, notif_rect_border_color.b, error_a);
    notif_img.ref.set_opt_color(raylib::Color(notif_img_color.r, notif_img_color.g, notif_img_color.b, error_a));
    notif_text.ref.set_opt_color(raylib::Color(255, 255, 255, error_a));
    notif_rect.ref.x.val = notif_rect_rel_x.get();
}

} // namespace editor
} // namespace scene
} // namespace kee