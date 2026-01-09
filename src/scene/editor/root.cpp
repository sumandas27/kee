#include "kee/scene/editor/root.hpp"

#include "kee/game.hpp"

namespace kee {
namespace scene {
namespace editor {

beatmap_file::beatmap_file(const beatmap_dir_state& dir_state, bool save_metadata_needed) :
    dir_state(dir_state),
    save_metadata_needed(save_metadata_needed)
{ }

confirm_save_ui::confirm_save_ui(const kee::ui::required& reqs, const kee::image_texture& error_png, root& root_elem, confirm_exit_ui& render_prio_owner) :
    kee::ui::rect(reqs,
        kee::color(0, 0, 0, 230),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(kee::border::type::rel_h, 0.4f),
        true,
        ui::rect_outline(ui::rect_outline::type::rel_h, 0.02f, kee::color::red_raylib),
        ui::rect_roundness(ui::rect_roundness::type::rel_h, 0.05f, std::nullopt)
    ),
    root_elem(root_elem),
    render_prio_owner(render_prio_owner),
    leave_color(add_transition<kee::color>(kee::color(230, 41, 55))),
    save_color(add_transition<kee::color>(kee::color(230, 41, 55))),
    confirm_save_ui_base(add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(kee::border::type::rel_h, 0.15f),
        true
    )),
    warning_png(confirm_save_ui_base.ref.add_child<kee::ui::image>(std::nullopt,
        error_png, kee::color::red_raylib,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 0.03f),
            dim(dim::type::aspect, 1)
        ),
        false, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    confirm_save_text(confirm_save_ui_base.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.06f),
        pos(pos::type::rel, 0),
        ui::text_size(ui::text_size::type::rel_h, 0.2f),
        std::nullopt, false, assets.font_regular, "You have unsaved changes! Are you sure you want to exit?", false
    )),
    leave_button(confirm_save_ui_base.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.0f),
        pos(pos::type::rel, 0.7f),
        dims(
            dim(dim::type::rel, 0.45f),
            dim(dim::type::rel, 0.3f)
        ),
        false
    )),
    leave_rect(leave_button.ref.add_child<kee::ui::rect>(std::nullopt,
        kee::color::blank,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(kee::border::type::abs, 0),
        true,
        ui::rect_outline(ui::rect_outline::type::rel_h, 0.1f, leave_color.get()),
        ui::rect_roundness(ui::rect_roundness::type::rel_h, 0.1f, std::nullopt)
    )),
    leave_text(leave_button.ref.add_child<kee::ui::text>(std::nullopt,
        leave_color.get(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.6f),
        std::nullopt, true, assets.font_semi_bold, "LEAVE", false
    )),
    save_button(confirm_save_ui_base.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.55f),
        pos(pos::type::rel, 0.7f),
        dims(
            dim(dim::type::rel, 0.45f),
            dim(dim::type::rel, 0.3f)
        ),
        false
    )),
    save_rect(save_button.ref.add_child<kee::ui::rect>(std::nullopt,
        kee::color::blank,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(kee::border::type::abs, 0),
        true,
        ui::rect_outline(ui::rect_outline::type::rel_h, 0.1f, save_color.get()),
        ui::rect_roundness(ui::rect_roundness::type::rel_h, 0.1f, std::nullopt)
    )),
    save_text(save_button.ref.add_child<kee::ui::text>(std::nullopt,
        save_color.get(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.6f),
        std::nullopt, true, assets.font_semi_bold, "SAVE", false
    )),
    destruct_flag(false)
{
    leave_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->leave_color.set(std::nullopt, kee::color::dark_orange, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->leave_color.set(std::nullopt, kee::color(230, 41, 55), 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    leave_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        this->game_ref.queue_game_exit();
    };

    save_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->save_color.set(std::nullopt, kee::color::dark_orange, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->save_color.set(std::nullopt, kee::color(230, 41, 55), 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    save_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        this->root_elem.save();
        this->game_ref.queue_game_exit();
    };

    render_prio_owner.release_render_priority();
    take_render_priority();
}

confirm_save_ui::~confirm_save_ui()
{
    release_render_priority();
    render_prio_owner.take_render_priority();
}

bool confirm_save_ui::should_destruct() const
{
    return destruct_flag;
}

bool confirm_save_ui::on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    if (!is_mouse_l)
        return false;

    if (!get_raw_rect().CheckCollision(mouse_pos))
        destruct_flag = true;

    return true;
}

void confirm_save_ui::update_element([[maybe_unused]] float dt)
{
    leave_rect.ref.outline.value().color = leave_color.get();
    leave_text.ref.color = leave_color.get();

    save_rect.ref.outline.value().color = save_color.get();
    save_text.ref.color = save_color.get();
}

confirm_exit_ui::confirm_exit_ui(const kee::ui::required& reqs, root& root_elem, const kee::image_texture& error_png, float menu_width) :
    kee::ui::base(reqs,
        pos(pos::type::end, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::abs, 0),
            dim(dim::type::rel, 1)
        ),
        false
    ),
    confirm_button(add_child<kee::ui::button>(-1,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 0.5f),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    error_png(error_png),
    base_w(add_transition<float>(0)),
    confirm_button_color(add_transition<kee::color>(kee::color(200, 0, 0))),
    go_back_button_color(add_transition<kee::color>(kee::color(0, 200, 0))),
    confirm_button_bg(confirm_button.ref.add_child<kee::ui::rect>(std::nullopt,
        confirm_button_color.get(),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        border(border::type::abs, 0),
        true, std::nullopt, std::nullopt
    )),
    confirm_button_text(confirm_button_bg.ref.make_temp_child<kee::ui::text>(
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.75f),
        std::nullopt, true, assets.font_semi_bold, "CONFIRM", false
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
        go_back_button_color.get(),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        border(border::type::abs, 0),
        true, std::nullopt, std::nullopt
    )),
    go_back_button_text(go_back_button_bg.ref.make_temp_child<kee::ui::text>(
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.75f),
        std::nullopt, true, assets.font_semi_bold, "GO BACK", false
    )),
    root_elem(root_elem)
{
    take_render_priority();
    base_w.set(get_raw_rect().height, menu_width, confirm_exit_ui::transition_time, kee::transition_type::exp);

    confirm_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->confirm_button_color.set(std::nullopt, kee::color(150, 0, 0), 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->confirm_button_color.set(std::nullopt, kee::color(200, 0, 0), 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    confirm_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (this->destroy_timer.has_value())
            return;

        if (this->root_elem.needs_save(this->root_elem.get_save_info()))
            this->confirm_save.emplace(this->root_elem.add_child<confirm_save_ui>(2, this->error_png, this->root_elem, *this));
        else
            this->game_ref.queue_game_exit();
    };

    go_back_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->go_back_button_color.set(std::nullopt, kee::color(0, 150, 0), 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->go_back_button_color.set(std::nullopt, kee::color(0, 200, 0), 0.5f, kee::transition_type::exp);
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
    confirm_button_bg.ref.color = confirm_button_color.get();
    go_back_button_bg.ref.color = go_back_button_color.get();

    if (confirm_save.has_value() && confirm_save.value().ref.should_destruct())
        confirm_save.reset();
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

song_ui::song_ui(const kee::ui::required& reqs, const kee::image_texture& arrow_png, const std::filesystem::path& music_path) :
    song_ui(reqs, arrow_png, raylib::Music(music_path.string()), 60.f, 0.f)
{ }

song_ui::song_ui(const kee::ui::required& reqs, const kee::image_texture& arrow_png, raylib::Music&& music, float music_bpm, float music_start_offset) :
    kee::ui::base(reqs,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0),
        true
    ),
    music_bpm(music_bpm),
    music_start_offset(music_start_offset),
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
        assets.play_png, kee::color::white,
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    music_time_text(add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::end, 0),
        pos(pos::type::end, 0),
        ui::text_size(ui::text_size::type::rel_h, 0.75f),
        std::nullopt, false, assets.font_regular, std::string(), false
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
        kee::color(20, 20, 20),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        border(border::type::rel_h, 0),
        false, std::nullopt, std::nullopt
    )),
    playback_l_img(playback_l_button.ref.add_child<kee::ui::image>(1,
        arrow_png, kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.15f),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    playback_speed_idx(3),
    playback_text_bg(add_child<kee::ui::rect>(std::nullopt,
        kee::color(10, 10, 10),
        pos(pos::type::rel, 0.07f),
        pos(pos::type::rel, 0.3f),
        dims(
            dim(dim::type::rel, 0.06f),
            dim(dim::type::rel, 0.7f)
        ),
        false, std::nullopt, std::nullopt
    )),
    playback_text(playback_text_bg.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.7f),
        std::nullopt, true, assets.font_semi_bold, std::format("{:.2f}x", song_ui::playback_speeds[playback_speed_idx]), false
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
        kee::color(20, 20, 20),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        border(border::type::rel_h, 0),
        false, std::nullopt, std::nullopt
    )),
    playback_r_img(playback_r_button.ref.add_child<kee::ui::image>(std::nullopt,
        arrow_png, kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.15f),
        true, ui::image::display::shrink_to_fit, true, false, 0.0f
    )),
    music(std::move(music)),
    music_time(0.0f)
{
    music_slider.ref.on_event = [&, music_is_playing = this->music.IsPlaying()](ui::slider::event slider_event) mutable
    {
        switch (slider_event)
        {
        case ui::slider::event::on_down:
            music_is_playing = this->music.IsPlaying();
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

    const unsigned int music_length = static_cast<unsigned int>(this->music.GetTimeLength());
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

    this->music.SetLooping(true);
    this->music.SetVolume(0.1f);
    /**
     * First `.Play()` on a `raylib::Music` is slow.
     */
    this->music.Play();
    this->music.Pause();
}

const raylib::Music& song_ui::get_music() const
{
    return music;
}

const std::optional<float>& song_ui::get_prev_beat() const
{
    return prev_beat;
}

float song_ui::get_time() const
{
    return music_time;
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
    pause_play_img.ref.color = pause_play_color.get();

    const unsigned int music_length = static_cast<unsigned int>(music.GetTimeLength());
    const unsigned int music_time_int = static_cast<unsigned int>(music_time);
    const std::string music_time_str = std::format("{}:{:02} / {}:{:02}", music_time_int / 60, music_time_int % 60, music_length / 60, music_length % 60);
    music_time_text.ref.set_string(music_time_str);

    const float playback_l_scale = (playback_speed_idx > 0) ? playback_l_img_scale.get() : 0.7f;
    const float playback_r_scale = (playback_speed_idx < song_ui::playback_speeds.size() - 1) ? playback_r_img_scale.get() : 0.7f;
    std::get<kee::border>(playback_l_img.ref.dimensions).val = (1.0f - playback_l_scale) / 2;
    std::get<kee::border>(playback_r_img.ref.dimensions).val = (1.0f - playback_r_scale) / 2;

    playback_l_img.ref.color = (playback_speed_idx > 0) ? playback_l_img_color.get() : kee::color(255, 255, 255, 80);
    playback_r_img.ref.color = (playback_speed_idx < song_ui::playback_speeds.size() - 1) ? playback_r_img_color.get() : kee::color(255, 255, 255, 80);
}

editor_hit_object_duration::editor_hit_object_duration(float duration, std::string_view hitsound_name) :
    duration(duration),
    hitsound_name(hitsound_name)
{
    assert(duration != 0.f);
}

editor_hit_object::editor_hit_object(
    int key,
    const std::string& hitsound_name, 
    const std::optional<editor_hit_object_duration>& hold_info
) :
    key(key),
    hitsound_name("normal.wav"),
    hold_info(duration != 0.f
        ? std::make_optional(editor_hit_object_duration(duration, "normal.wav"))
        : std::nullopt
    )
{ }

float editor_hit_object::get_duration() const
{
    return hold_info.has_value() ? hold_info.value().duration : 0.f;
}

beatmap_save_info::beatmap_save_info(bool need_save_metadata, bool need_save_song, bool need_save_img, bool need_save_vid, bool need_save_vid_offset, bool need_save_hit_objs, bool need_save_key_color, bool need_save_hitsound) :
    need_save_metadata(need_save_metadata),
    need_save_song(need_save_song),
    need_save_img(need_save_img),
    need_save_vid(need_save_vid),
    need_save_vid_offset(need_save_vid_offset),
    need_save_hit_objs(need_save_hit_objs),
    need_save_key_color(need_save_key_color),
    need_save_hitsound(need_save_hitsound)
{ }

image_state::image_state(const std::filesystem::path& path) :
    path(path),
    texture(kee::image_texture(path))
{ }

video_state::video_state(const std::filesystem::path& path, float offset) :
    path(path),
    offset(offset)
{ }

key_color_state::key_color_state()
{
    for (const key_pos_data& key_data : kee::key_ui_data)
    {
        const std::string key_str = std::string(1, static_cast<char>(key_data.raylib_key));
        decorations[key_str] = std::vector<key_decoration>();
    }
}

key_color_state::key_color_state(const std::filesystem::path& path, const boost::json::object& json) :
    path(path)
{ 
    for (const auto& [key, val] : json)
        decorations[key] = beatmap_dir_info::get_key_decorations(val.as_array());
}

key_color_state::key_color_state(const std::filesystem::path& path, root& root_elem) :
    path(path)
{ 
    const auto parsed_json = beatmap_dir_info::parse_key_colors(path);
    if (parsed_json.has_value())
        for (const auto& [key, val] : parsed_json.value())
            decorations[key] = beatmap_dir_info::get_key_decorations(val.as_array());
    else
        root_elem.set_error(parsed_json.error(), true);
}

hitsound_state::hitsound_state()
{
    static const std::filesystem::path default_hitsound_path = "assets/sfx/hitsound_default";
    for (const std::filesystem::directory_entry& hitsound_wav : std::filesystem::directory_iterator(default_hitsound_path))
        map[hitsound_wav.path().filename().string()] = raylib::Sound(hitsound_wav.path().string());

    /* TODO: hard coded don't like */
    map.at("normal.wav").SetVolume(0.01f);
}

hitsound_state::hitsound_state(const std::filesystem::path& path, std::unordered_map<std::string, raylib::Sound>&& map) :
    path(path),
    map(std::move(map))
{ }

hitsound_state::hitsound_state(const std::filesystem::path& path, root& root_elem) :
    path(path)
{
    auto custom_hitsounds = beatmap_dir_info::validate_custom_hitsounds(path);
    if (custom_hitsounds.has_value())
        map = std::move(custom_hitsounds.value());
    else
        root_elem.set_error(custom_hitsounds.error(), true);
}

root::root(kee::game& game, kee::global_assets& assets, const std::optional<std::filesystem::path>& beatmap_dir_name) :
    root(game, assets, beatmap_dir_name.has_value() 
        ? std::make_optional(beatmap_dir_info(beatmap_dir_name.value()))
        : std::nullopt
    )
{ }

std::optional<beatmap_save_info> root::get_save_info() const
{
    const bool save_hit_objs_needed = (!compose_info.events_since_save.has_value() || compose_info.events_since_save.value() != 0);
    if (!save_state.has_value())
        return std::nullopt;

    bool need_img_save;
    if (!setup_info.img_state.has_value() && !save_state.value().dir_state.has_image)
        need_img_save = false;
    else if (setup_info.img_state.has_value() && save_state.value().dir_state.has_image)
        need_img_save = !std::filesystem::equivalent(setup_info.img_state.value().path, save_state.value().dir_state.path / beatmap_dir_state::standard_img_filename);
    else
        need_img_save = true;

    bool need_vid_save;
    if (!vid_state.has_value() && !save_state.value().dir_state.video_dir_info.has_value())
        need_vid_save = false;
    else if (vid_state.has_value() && save_state.value().dir_state.video_dir_info.has_value())
        need_vid_save = !std::filesystem::equivalent(vid_state.value().path, save_state.value().dir_state.path / beatmap_dir_state::standard_vid_filename);
    else
        need_vid_save = true;

    bool need_save_key_color;
    if (!key_colors.path.has_value() && !save_state.value().dir_state.has_key_colors)
        need_save_key_color = false;
    else if (key_colors.path.has_value() && save_state.value().dir_state.has_key_colors)
        need_save_key_color = !std::filesystem::equivalent(key_colors.path.value(), save_state.value().dir_state.path / beatmap_dir_state::standard_key_colors_filename);
    else
        need_save_key_color = true;

    bool need_save_hitsound;
    if (!hitsounds.path.has_value() && !save_state.value().dir_state.has_custom_hitsounds)
        need_save_hitsound = false;
    else if (hitsounds.path.has_value() && save_state.value().dir_state.has_custom_hitsounds)
        need_save_hitsound = !std::filesystem::equivalent(hitsounds.path.value(), save_state.value().dir_state.path / beatmap_dir_state::standard_custom_hitsound_dirname);
    else
        need_save_hitsound = true;

    const bool need_save_vid_offset = need_vid_save || (vid_state.has_value() && save_state.value().dir_state.video_dir_info.has_value() && vid_state.value().offset != save_state.value().dir_state.video_dir_info.value());
    return beatmap_save_info(save_state.value().save_metadata_needed, setup_info.new_song_path.has_value(), need_img_save, need_vid_save, need_save_vid_offset, save_hit_objs_needed, need_save_key_color, need_save_hitsound);
}

bool root::needs_save(const std::optional<beatmap_save_info>& save_info) const
{
    return !save_info.has_value() || (
        save_info.value().need_save_metadata ||
        save_info.value().need_save_song ||
        save_info.value().need_save_img ||
        save_info.value().need_save_vid ||
        save_info.value().need_save_vid_offset ||
        save_info.value().need_save_hit_objs ||
        save_info.value().need_save_key_color ||
        save_info.value().need_save_hitsound
    );
}

void root::save()
{
    if (this->save_state.has_value())
    {
        this->save_existing_beatmap();
        return;
    }

    if (!setup_info.new_song_path.has_value())
    {
        this->set_error("Choose a song before saving!", false);
        return;
    }

    std::error_code ec;
    const std::filesystem::path new_path = beatmap_dir_info::app_data_dir / "local_0";
    const bool success = std::filesystem::create_directory(new_path, ec);

    if (ec)
    {
        this->set_error(std::format("Error saving: {}", ec.message()), false);
        return;
    }

    if (!success)
        throw std::runtime_error("Directory already exists");

    this->save_state.emplace(beatmap_dir_state(new_path), true);
    this->save_existing_beatmap();
}

void root::save_existing_beatmap()
{
    const std::optional<beatmap_save_info> save_info = get_save_info();
    if (!save_info.has_value())
        throw std::runtime_error("Saving beatmap when no directory for it exists.");

    if (save_info.value().need_save_song)
    {
        std::error_code ec;
        const std::filesystem::path& src = setup_info.new_song_path.value();
        const std::filesystem::path dst = save_state.value().dir_state.path / "song.mp3";
        
        std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec);
        if (ec)
        {
            set_error(std::format("Error saving song: {}", ec.message()), false);
            return;
        }

        setup_info.new_song_path.reset();
    }

    if (save_info.value().need_save_hitsound)
    {
        if (hitsounds.path.has_value())
        {
            std::error_code ec;
            const std::filesystem::path& src = hitsounds.path.value();
            const std::filesystem::path dst = save_state.value().dir_state.path / beatmap_dir_state::standard_custom_hitsound_dirname;

            std::filesystem::copy(src, dst, std::filesystem::copy_options::recursive, ec);
            if (ec)
            {
                set_error(std::format("Error saving hitsounds: {}", ec.message()), false);
                return;
            }

            save_state.value().dir_state.has_custom_hitsounds = true;
            hitsounds.path = dst;
        }
        else
        {
            std::error_code ec;
            std::filesystem::remove_all(save_state.value().dir_state.path / beatmap_dir_state::standard_custom_hitsound_dirname, ec);
            if (ec)
                throw std::runtime_error(std::format("Failed to remove beatmap's hitsounds! {}", ec.message()));

            save_state.value().dir_state.has_custom_hitsounds = false;
        }
    }

    if (save_info.value().need_save_img)
    {
        if (setup_info.img_state.has_value())
        {
            std::error_code ec;
            const std::filesystem::path& src = setup_info.img_state.value().path;
            const std::filesystem::path dst = save_state.value().dir_state.path / beatmap_dir_state::standard_img_filename;

            std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec);
            if (ec)
            {
                set_error(std::format("Error saving beatmap image: {}", ec.message()), false);
                return;
            }

            save_state.value().dir_state.has_image = true;
            setup_info.img_state.value().path = dst;
        }
        else 
        {
            if (!std::filesystem::remove(save_state.value().dir_state.path / beatmap_dir_state::standard_img_filename))
                throw std::runtime_error("Failed to remove beatmap's image file!");

            save_state.value().dir_state.has_image = false;
        }
    }

    if (save_info.value().need_save_vid)
    {
        if (vid_state.has_value())
        {
            std::error_code ec;
            const std::filesystem::path& src = vid_state.value().path;
            const std::filesystem::path dst = save_state.value().dir_state.path / beatmap_dir_state::standard_vid_filename;

            std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec);
            if (ec)
            {
                set_error(std::format("Error saving beatmap video: {}", ec.message()), false);
                return;
            }

            save_state.value().dir_state.video_dir_info = vid_state.value().offset;
            vid_state.value().path = dst;
        }
        else
        {
            if (!std::filesystem::remove(save_state.value().dir_state.path / beatmap_dir_state::standard_vid_filename))
                throw std::runtime_error("Failed to remove beatmap's video file!");
        
            save_state.value().dir_state.video_dir_info.reset();
        }
    }
    else if (save_info.value().need_save_vid_offset)
        save_state.value().dir_state.video_dir_info = vid_state.value().offset;

    if (save_info.value().need_save_key_color)
    {
        if (key_colors.path.has_value())
        {
            std::error_code ec;
            const std::filesystem::path& src = key_colors.path.value();
            const std::filesystem::path dst = save_state.value().dir_state.path / beatmap_dir_state::standard_key_colors_filename;

            std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec);
            if (ec)
            {
                set_error(std::format("Error saving key colors: {}", ec.message()), false);
                return;
            }

            save_state.value().dir_state.has_key_colors = true;
            key_colors.path.value() = dst;
        }
        else
        {
            if (!std::filesystem::remove(save_state.value().dir_state.path / beatmap_dir_state::standard_key_colors_filename))
                throw std::runtime_error("Failed to remove key color decoration file!");

            save_state.value().dir_state.has_key_colors = false;
        }
    }

    if (save_info.value().need_save_metadata || save_info.value().need_save_hit_objs || save_info.value().need_save_vid_offset)
    {
        boost::json::object output;
        output["song_artist"] = setup_info.song_artist;
        output["song_name"] = setup_info.song_name;
        output["mapper"] = setup_info.mapper;
        output["level_name"] = setup_info.level_name;

        output["beat_forgiveness"] = setup_info.beat_forgiveness;
        output["approach_beats"] = approach_beats;

        const song_ui& song_info = std::get<kee::ui::handle<song_ui>>(playback_ui).ref;
        output["song_bpm"] = song_info.music_bpm;
        output["song_start_offset"] = song_info.music_start_offset;

        if (save_state.value().dir_state.video_dir_info.has_value())
            output["video_offset"] = save_state.value().dir_state.video_dir_info.value();

        boost::json::object& hit_objects = output["hit_objects"].emplace_object();
        for (int key : compose_tab::prio_to_key)
        {
            boost::json::array key_hit_objs;
            for (const auto& [beat, hit_obj] : compose_info.hit_objs[key])
            {
                boost::json::object json_hit_obj;
                json_hit_obj["beat"] = beat;
                json_hit_obj["duration"] = hit_obj.get_duration();

                key_hit_objs.push_back(json_hit_obj);
            }

            const std::string key_str = std::string(1, static_cast<char>(key));
            hit_objects[key_str] = std::move(key_hit_objs);
        }

        const std::filesystem::path metadata_path = save_state.value().dir_state.path / "metadata.json";
        std::ofstream metadata_file = std::ofstream(metadata_path, std::ios::binary | std::ios::trunc);
        if (!metadata_file)
        {
            set_error("Unable to save metadata file!", false);
            return;
        }

        const std::string json_serialized = boost::json::serialize(output);
        metadata_file << json_serialized;

        save_state.value().save_metadata_needed = false;
        compose_info.events_since_save = 0;
    }

    if (needs_save(save_info))
        set_info("Beatmap saved");
}

void root::set_error(std::string_view error_str, bool from_file_dialog)
{
    const kee::color new_notif_color = kee::color(255, 0, 0, notif_img.ref.color.a);

    notif_img.ref.set_image(error_png);
    notif_img.ref.color = new_notif_color;
    notif_rect.ref.outline.value().color = new_notif_color;
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
    const kee::color old_notif_color = notif_img.ref.color;
    const kee::color new_notif_color = kee::color(144, 213, 255, old_notif_color.a);

    notif_img.ref.set_image(info_png);
    notif_img.ref.color = new_notif_color;
    notif_rect.ref.outline.value().color = new_notif_color;
    notif_text.ref.set_string(info_str);

    notif_skips_before_start = 0;
}


void root::set_song(const std::filesystem::path& song_path)
{
    playback_ui.emplace<kee::ui::handle<song_ui>>(playback_ui_frame.ref.add_child<song_ui>(std::nullopt, arrow_png, song_path));
}

root::root(kee::game& game, kee::global_assets& assets, std::optional<beatmap_dir_info> dir_info) :
    kee::scene::base(game, assets),
    save_state(dir_info.has_value() 
        ? std::make_optional(beatmap_file(dir_info.value().dir_state, false)) 
        : std::nullopt
    ),
    arrow_png("assets/img/arrow.png"),
    error_png("assets/img/error.png"),
    exit_png("assets/img/exit.png"),
    info_png("assets/img/info.png"),
    img_state(dir_info.has_value() && dir_info.value().dir_state.has_image
        ? std::make_optional(image_state(dir_info.value().dir_state.path / beatmap_dir_state::standard_img_filename))
        : std::nullopt
    ),
    vid_state(dir_info.has_value() && dir_info.value().dir_state.video_dir_info.has_value()
        ? std::make_optional(video_state(dir_info.value().dir_state.path / beatmap_dir_state::standard_vid_filename, dir_info.value().dir_state.video_dir_info.value()))
        : std::nullopt
    ),
    key_colors(dir_info.has_value() && dir_info.value().key_colors_json_obj.has_value()
        ? key_color_state(dir_info.value().dir_state.path / beatmap_dir_state::standard_key_colors_filename, dir_info.value().key_colors_json_obj.value())
        : key_color_state()
    ),
    hitsounds(dir_info.has_value() && dir_info.value().custom_hitsounds.has_value()
        ? hitsound_state(dir_info.value().dir_state.path / beatmap_dir_state::standard_custom_hitsound_dirname, std::move(dir_info.value().custom_hitsounds.value()))
        : hitsound_state()
    ),
    approach_beats(dir_info.has_value() ? dir_info.value().approach_beats : 2.0f),
    setup_info(dir_info, exit_png, img_state, vid_state, key_colors, hitsounds),
    compose_info(
        arrow_png,
        dir_info.has_value()
            ? std::make_optional(dir_info.value().keys_json_obj) 
            : std::nullopt,
        img_state, vid_state, key_colors, hitsounds
    ),
    active_tab_elem(add_child<setup_tab>(std::nullopt, *this, setup_info, approach_beats)),
    active_tab(root::tabs::setup),
    tab_active_rect_rel_x(add_transition<float>(static_cast<float>(active_tab) / magic_enum::enum_count<root::tabs>())),
    exit_button_rect_alpha(add_transition<float>(0.0f)),
    notif_rect_rel_x(add_transition<float>(1.0f)),
    notif_alpha(add_transition<float>(0)),
    tab_rect(add_child<kee::ui::rect>(1,
        kee::color(10, 10, 10, 255),
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
        kee::color(20, 20, 20, 255),
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
        kee::color(255, 0, 0, static_cast<unsigned char>(exit_button_rect_alpha.get())),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0),
        true, std::nullopt, std::nullopt
    )),
    exit_button_image(exit_button.ref.add_child<kee::ui::image>(1,
        exit_png, kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_w, 0.3f),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    playback_bg(add_child<kee::ui::rect>(std::nullopt,
        kee::color(30, 30, 30),
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
    playback_ui(dir_info.has_value() ?
        std::variant<kee::ui::handle<song_ui>, kee::ui::handle<kee::ui::text>>(
            playback_ui_frame.ref.add_child<song_ui>(std::nullopt, 
                arrow_png, 
                std::move(dir_info.value().song), 
                dir_info.value().song_bpm, 
                dir_info.value().song_start_offset
            )
        ) :
        std::variant<kee::ui::handle<song_ui>, kee::ui::handle<kee::ui::text>>(
            playback_ui_frame.ref.add_child<kee::ui::text>(std::nullopt,
                kee::color::white,
                pos(pos::type::rel, 0),
                pos(pos::type::rel, 0),
                ui::text_size(ui::text_size::type::rel_h, 0.5f),
                std::nullopt, false, assets.font_semi_bold, "NO SONG SELECTED", false
            )
        )
    ),
    notif_rect(add_child<kee::ui::rect>(1,
        kee::color(80, 80, 80, static_cast<unsigned char>(notif_alpha.get())),
        pos(pos::type::rel, notif_rect_rel_x.get()),
        pos(pos::type::rel, 0.93f),
        dims(
            dim(dim::type::rel, 0.2f),
            dim(dim::type::rel, 0.05f)
        ),
        false,
        ui::rect_outline(ui::rect_outline::type::rel_h, 0.1f, kee::color(255, 0, 0, static_cast<unsigned char>(notif_alpha.get()))),
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
        error_png, kee::color(255, 0, 0, notif_alpha.get()),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.3f),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    notif_text(notif_rect.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(255, 255, 255, static_cast<unsigned char>(notif_alpha.get())),
        pos(pos::type::rel, 0.13f),
        pos(pos::type::rel, 0.3f),
        ui::text_size(ui::text_size::type::rel_h, 0.4f),
        kee::dim(kee::dim::type::rel, 0.82f),
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
        this->confirm_exit.emplace(this->exit_button.ref.add_child<confirm_exit_ui>(2, *this, this->error_png, this->tab_rect.ref.get_raw_rect().width));
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
            kee::color::white,
            pos(pos::type::rel, 0.5f),
            pos(pos::type::rel, 0.5f),
            ui::text_size(ui::text_size::type::rel_h, 0.6f),
            std::nullopt, true, assets.font_semi_bold, enum_name, false
        ));
    }
}

bool root::on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods)
{
    switch (keycode)
    {
    case KeyboardKey::KEY_SPACE:
        if (mods.test(kee::mods::ctrl) || !std::holds_alternative<kee::ui::handle<song_ui>>(playback_ui))
            return false;

        std::get<kee::ui::handle<song_ui>>(playback_ui).ref.pause_play_click(mods);
        return true;
    case KeyboardKey::KEY_ESCAPE:
        if (!confirm_exit.has_value())
            exit_button.ref.on_click_l(mods);
        else if (!confirm_exit.value().ref.confirm_save.has_value())
            confirm_exit.value().ref.confirm_button.ref.on_click_l(mods);
        else
            game_ref.queue_game_exit();

        return true;
    case KeyboardKey::KEY_S: {
        if (!mods.test(kee::mods::ctrl))
            return false;

        this->save();
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
        tab_button_text[i].ref.color = tab_button_text_colors[i].get().get();

    kee::color exit_button_rect_color = exit_button_rect.ref.color;
    exit_button_rect_color.a = static_cast<unsigned char>(exit_button_rect_alpha.get());
    exit_button_rect.ref.color = exit_button_rect_color;
    tab_active_rect.ref.x.val = tab_active_rect_rel_x.get();

    if (confirm_exit.has_value() && confirm_exit.value().ref.should_destruct())
        confirm_exit.reset();

    const unsigned char error_a = static_cast<unsigned char>(notif_alpha.get());
    const kee::color notif_rect_border_color = notif_rect.ref.outline.value().color;
    const kee::color notif_img_color = notif_img.ref.color;
    
    notif_rect.ref.color = kee::color(80, 80, 80, error_a);
    notif_rect.ref.outline.value().color = kee::color(notif_rect_border_color.r, notif_rect_border_color.g, notif_rect_border_color.b, error_a);
    notif_img.ref.color = kee::color(notif_img_color.r, notif_img_color.g, notif_img_color.b, error_a);
    notif_text.ref.color = kee::color(255, 255, 255, error_a);
    notif_rect.ref.x.val = notif_rect_rel_x.get();
}

} // namespace editor
} // namespace scene
} // namespace kee