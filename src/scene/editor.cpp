#include "kee/scene/editor.hpp"

#include "kee/ui/rect.hpp"
#include "kee/ui/slider.hpp"
#include "kee/ui/text.hpp"
#include "kee/ui/triangle.hpp"

namespace kee {
namespace scene {

editor::editor(const kee::scene::window& window, kee::global_assets& assets) :
    kee::scene::base(window, assets),
    beat_step(0.25f),
    music_start_offset(0.5f),
    music_bpm(100.0f),
    id_trans_pause_play_color(0),
    id_trans_pause_play_scale(1),
    play_png("assets/img/play.png"),
    pause_png("assets/img/pause.png"),
    mouse_wheel_move(0.0f),
    is_music_playing(true),
    music("assets/daft-punk-something-about-us/daft-punk-something-about-us.mp3")
{
    id_beat_indicator = add_child_no_id<kee::ui::triangle>(
        raylib::Color::Red(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.02f),
        dims(
            dim(dim::type::rel, 0.02f),
            dim(dim::type::rel, 0.02f)
        ),
        raylib::Vector2(0, 0),
        raylib::Vector2(1, 0),
        raylib::Vector2(0.5, 1),
        kee::ui::common(true, 0, false)
    );

    id_beat_ticks_frame = add_child_no_id<kee::ui::base>(
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.05f),
        dims(
            dim(dim::type::rel, 0.95f),
            dim(dim::type::rel, 0.05f)
        ),
        kee::ui::common(true, 1, false)
    );

    id_music_slider = add_child_no_id<kee::ui::slider>(
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.125f),
        dims(
            dim(dim::type::rel, 0.95f),
            dim(dim::type::rel, 0.005f)
        ),
        true, std::nullopt
    );

    auto& music_slider = *dynamic_cast<kee::ui::slider*>(child_at(id_music_slider).get());
    music_slider.on_event = [&](ui::slider::event slider_event)
    {
        switch (slider_event)
        {
        case ui::slider::event::on_down:
            this->active_child = music_slider;
            this->music.Pause();
            break;
        case ui::slider::event::on_release:
            this->active_child = boost::none;
            this->music.Seek(music_slider.progress * this->music.GetTimeLength());
            if (this->is_music_playing)
                this->music.Resume();
            break;
        }
    };

    id_pause_play = child_at(id_music_slider)->add_child_no_id<kee::ui::button>(
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 60),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::abs, 60)
        ),
        kee::ui::common(false, std::nullopt, false)
    );

    auto& pause_play = *dynamic_cast<kee::ui::button*>(child_at(id_music_slider)->child_at(id_pause_play).get());
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

    pause_play.on_event = [&](ui::button::event button_event)
    {
        std::unordered_map<unsigned int, std::unique_ptr<kee::transition_base>>& transitions = this->child_at(id_music_slider)->child_at(id_pause_play)->transitions;
        auto& color_transition = *dynamic_cast<kee::transition<kee::color>*>(transitions[id_trans_pause_play_color].get());
        auto& scale_transition = *dynamic_cast<kee::transition<float>*>(transitions[id_trans_pause_play_scale].get());

        switch (button_event)
        {
        case ui::button::event::on_hot:
            color_transition.set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
            scale_transition.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down:
            scale_transition.set(std::nullopt, 0.9f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            color_transition.set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
            scale_transition.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            break;
        }
    };

    pause_play.on_click = [&]()
    { 
        this->is_music_playing = !this->is_music_playing;

        auto& pause_play_img = *dynamic_cast<kee::ui::image*>(this->child_at(id_music_slider)->child_at(id_pause_play)->child_at(id_pause_play_png).get());
        if (this->is_music_playing)
        {
            this->music.Seek(music_slider.progress * this->music.GetTimeLength());
            this->music.Resume();
            pause_play_img.set_image(pause_png);
        }
        else
        {
            this->music.Pause();
            pause_play_img.set_image(play_png);
        }
    };

    const unsigned int music_length = static_cast<unsigned int>(music.GetTimeLength());
    const std::string music_length_str = std::format("0:00 / {}:{:02}", music_length / 60, music_length % 60);

    id_music_time_text = child_at(id_music_slider)->add_child_no_id<kee::ui::text>(
        raylib::Color::White(),
        pos(pos::type::end, 0),
        pos(pos::type::beg, 50),
        ui::text_size(ui::text_size::type::abs, 80),
        assets.font_regular, music_length_str, false,
        kee::ui::common(false, std::nullopt, false)
    );

    id_key_border = add_child_no_id<kee::ui::base>(
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.55f),
        dims(
            dim(dim::type::rel, 0.9f),
            dim(dim::type::rel, 0.6f)
        ),
        kee::ui::common(true, std::nullopt, false)
    );

    id_key_frame = child_at(id_key_border)->add_child_no_id<kee::ui::base>(
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 10),
            dim(dim::type::aspect, 4)
        ),
        kee::ui::common(true, std::nullopt, false)
    );

    for (const auto& [id, rel_pos] : kee::key_ui_data)
    {
        child_at(id_key_border)->child_at(id_key_frame)->add_child_with_id<kee::ui::base>(id,
            pos(pos::type::rel, rel_pos.x),
            pos(pos::type::rel, rel_pos.y),
            dims(
                dim(dim::type::aspect, id == KeyboardKey::KEY_SPACE ? 7.0f : 1.0f),
                dim(dim::type::rel, 0.25)
            ),
            kee::ui::common(true, std::nullopt, false)
        );

        id_editor_key = child_at(id_key_border)->child_at(id_key_frame)->child_at(id)->add_child_no_id<editor_key>(*this, id);
    }

    music.SetLooping(true);
    music.SetVolume(0.1f);

    if (is_music_playing)
        music.Play();
}

void editor::select_key(int id, bool clear)
{
    if (clear)
    {
        for (int prev_id : selected_key_ids)
            child_at(id_key_border)->child_at(id_key_frame)->child_at(prev_id)->child_at(id_editor_key)->set_opt_color(raylib::Color::White());

        selected_key_ids.clear();
    }

    selected_key_ids.push_back(id);
    child_at(id_key_border)->child_at(id_key_frame)->child_at(id)->child_at(id_editor_key)->set_opt_color(raylib::Color::Green());
}

void editor::handle_element_events()
{
    if (raylib::Keyboard::IsKeyPressed(KeyboardKey::KEY_SPACE))
    {
        if (raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_LEFT_CONTROL))
        {
            for (int prev_id : selected_key_ids)
                child_at(id_key_border)->child_at(id_key_frame)->child_at(prev_id)->child_at(id_editor_key)->set_opt_color(raylib::Color::White());

            selected_key_ids.clear();
        }
        else
        {
            auto& pause_play = *dynamic_cast<kee::ui::button*>(child_at(id_music_slider)->child_at(id_pause_play).get());
            pause_play.on_click();
        }
    }

    if (is_music_playing)
        return;

    mouse_wheel_move += raylib::Mouse::GetWheelMove();
    if (std::abs(mouse_wheel_move) < 1.0f)
        return;

    auto& music_slider = *dynamic_cast<kee::ui::slider*>(child_at(id_music_slider).get());
    const float music_time = music_slider.progress * music.GetTimeLength();
    const float beat = (music_time - music_start_offset) * music_bpm / 60.0f;

    float new_beat = beat;
    float beat_steps = 0;

    if (mouse_wheel_move <= -1.0f)
    {
        beat_steps = -std::ceil(mouse_wheel_move);
        mouse_wheel_move += beat_steps;

        const float beat_floor = std::floorf(beat / beat_step) * beat_step;
        new_beat = (std::abs(beat_floor - beat) < editor::beat_lock_threshold) 
            ? beat_floor - beat_step * beat_steps 
            : beat_floor - beat_step * (beat_steps - 1);
    }
    else if (mouse_wheel_move >= 1.0f)
    {
        beat_steps = std::floor(mouse_wheel_move);
        mouse_wheel_move -= beat_steps;

        const float beat_ceil = std::ceilf(beat / beat_step) * beat_step;
        new_beat = (std::abs(beat_ceil - beat) < editor::beat_lock_threshold) 
            ? beat_ceil + beat_step * beat_steps 
            : beat_ceil + beat_step * (beat_steps - 1);
    }

    const float new_music_time = music_start_offset + new_beat * 60.0f / music_bpm;
    const float new_progress = (new_music_time / music.GetTimeLength());
    music_slider.progress = std::clamp(new_progress, 0.0f, 1.0f);
}

void editor::update_element([[maybe_unused]] float dt)
{
    auto& music_slider = *dynamic_cast<kee::ui::slider*>(child_at(id_music_slider).get());

    if (music.IsPlaying())
    {
        music.Update();
        music_slider.progress = music.GetTimePlayed() / music.GetTimeLength();
    }

    auto& pause_play = *dynamic_cast<kee::ui::button*>(this->child_at(id_music_slider)->child_at(id_pause_play).get());
    auto& pause_play_color_trans = *dynamic_cast<kee::transition<kee::color>*>(pause_play.transitions[id_trans_pause_play_color].get());
    auto& pause_play_scale_trans = *dynamic_cast<kee::transition<float>*>(pause_play.transitions[id_trans_pause_play_scale].get());
    
    auto& pause_play_png = *dynamic_cast<kee::ui::image*>(pause_play.child_at(id_pause_play_png).get());
    pause_play_png.set_opt_color(pause_play_color_trans.get().to_color());
    
    auto& [w, h] = std::get<kee::dims>(pause_play_png.dimensions);
    w.val = pause_play_scale_trans.get();
    h.val = pause_play_scale_trans.get();

    const unsigned int music_length = static_cast<unsigned int>(music.GetTimeLength());
    const unsigned int music_time = static_cast<unsigned int>(music_slider.progress * music.GetTimeLength());
    const std::string music_time_str = std::format("{}:{:02} / {}:{:02}", music_time / 60, music_time % 60, music_length / 60, music_length % 60);

    auto& music_time_text = *dynamic_cast<kee::ui::text*>(child_at(id_music_slider)->child_at(id_music_time_text).get());
    music_time_text.set_string(music_time_str);
}

void editor::render_element_behind_children() const
{
    static constexpr float fade_percent = 0.05f;
    static constexpr float beat_delta = 5.0f;

    const auto& music_slider = *dynamic_cast<const kee::ui::slider*>(child_at(id_music_slider).get());
    const float music_time = music_slider.progress * music.GetTimeLength();

    const float beat = (music_time - music_start_offset) * music_bpm / 60.0f;
    const float beat_start = std::ceil((beat - beat_delta) / beat_step) * beat_step;

    for (float beat_render = beat_start; beat_render <= beat + beat_delta; beat_render += beat_step)
    {
        const float render_rel_x = (beat_render - (beat - beat_delta)) / (2 * beat_delta);

        float opacity = 1.0f;
        if (render_rel_x <= fade_percent)
            opacity = render_rel_x / fade_percent;
        else if (render_rel_x >= 1.0f - fade_percent)
            opacity = (1.0f - render_rel_x) / fade_percent;

        const bool is_whole_beat = std::floorf(beat_render) == beat_render;
        const float render_rel_w = is_whole_beat ? 0.003f : 0.001f;
        const float render_rel_h = is_whole_beat ? 0.5f : 0.25f;

        const kee::ui::rect beat_render_rect = child_at(id_beat_ticks_frame)->make_temp_child<kee::ui::rect>(
            raylib::Color(255, 255, 255, static_cast<unsigned char>(255 * opacity)),
            pos(pos::type::rel, render_rel_x),
            pos(pos::type::rel, 0.25f),
            dims(
                dim(dim::type::rel, render_rel_w),
                dim(dim::type::rel, render_rel_h)
            ),
            std::nullopt,
            kee::ui::rect_roundness(kee::ui::rect_roundness::type::rel_w, 0.5f),
            kee::ui::common(true, std::nullopt, false)
        );

        beat_render_rect.render();

        if (is_whole_beat)
        {
            const int whole_beat = static_cast<int>(std::floorf(beat_render));
            const kee::ui::text whole_beat_text = child_at(id_beat_ticks_frame)->make_temp_child<kee::ui::text>(
                raylib::Color(255, 255, 255, static_cast<unsigned char>(255 * opacity)),
                pos(pos::type::rel, render_rel_x),
                pos(pos::type::rel, 0.95f),
                ui::text_size(ui::text_size::type::rel_h, 0.75f),
                assets.font_semi_bold, std::to_string(whole_beat), false,
                kee::ui::common(true, std::nullopt, false)
            );

            whole_beat_text.render();
        }
    }
}

editor_key::editor_key(const kee::ui::base::required& reqs, kee::scene::editor& editor_scene, int key_id) :
    kee::ui::button(reqs,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, kee::key_border_parent_h),
        kee::ui::common(true, std::nullopt, false)
    ),
    editor_scene(editor_scene),
    key_id(key_id),
    is_control_clicked(false)
{
    on_event = [&](ui::button::event button_event)
    {
        if (button_event == ui::button::event::on_down && raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_LEFT_CONTROL))
            is_control_clicked = true;
    };

    on_click = [&]()
    {
        this->editor_scene.select_key(this->key_id, !is_control_clicked);
    };

    set_opt_color(raylib::Color::White());

    add_child_no_id<kee::ui::rect>(
        raylib::Color::Blank(),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        ui::rect_outline(ui::rect_outline::type::rel_h, kee::key_border_width, std::nullopt), 
        std::nullopt,
        kee::ui::common(true, std::nullopt, true)
    );

    const std::string key_str = (key_id != KeyboardKey::KEY_SPACE) 
        ? std::string(1, static_cast<char>(key_id)) 
        : "___";

    add_child_no_id<kee::ui::text>(
        std::nullopt,
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        ui::text_size(ui::text_size::type::rel_h, 0.5f),
        assets.font_light, key_str, false, 
        kee::ui::common(true, 0, false)
    );
}

void editor_key::handle_element_events()
{
    kee::ui::button::handle_element_events();

    if (is_control_clicked && !raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_LEFT_CONTROL))
        is_control_clicked = false;
}

} // namespace scene
} // namespace kee