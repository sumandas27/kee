#include "kee/scene/editor.hpp"

namespace kee {
namespace scene {

editor::editor(const kee::scene::window& window, kee::global_assets& assets) :
    kee::scene::base(window, assets),
    approach_beats(2.0f),
    beat_step(0.25f),
    play_png("assets/img/play.png"),
    pause_png("assets/img/pause.png"),
    pause_play_color(add_transition<kee::color>(kee::color::white())),
    pause_play_scale(add_transition<float>(1.0f)),
    obj_editor(add_child<object_editor>(*this, keys, selected_key_ids)),
    music_slider(add_child<kee::ui::slider>(
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.12f),
        dims(
            dim(dim::type::rel, 0.95f),
            dim(dim::type::rel, 0.005f)
        ),
        true, std::nullopt
    )),
    pause_play(music_slider.add_child<kee::ui::button>(
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 40),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::abs, 60)
        ),
        kee::ui::common(false, std::nullopt, false)
    )),
    pause_play_img(pause_play.add_child<kee::ui::image>(
        pause_png,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        kee::ui::common(true, std::nullopt, false)
    )),
    music_time_text(music_slider.add_child<kee::ui::text>(
        raylib::Color::White(),
        pos(pos::type::end, 0),
        pos(pos::type::beg, 30),
        ui::text_size(ui::text_size::type::abs, 80),
        assets.font_regular, std::string(), false,
        kee::ui::common(false, std::nullopt, false)
    )),
    key_border(add_child<kee::ui::base>(
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::rel, 0.7f),
            dim(dim::type::rel, 0.5f)
        ),
        kee::ui::common(true, std::nullopt, false)
    )),
    key_frame(key_border.add_child<kee::ui::base>(
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 10),
            dim(dim::type::aspect, 4)
        ),
        kee::ui::common(true, std::nullopt, false)
    )),
    music_start_offset(0.5f),
    music_bpm(100.0f),
    mouse_wheel_move(0.0f),
    music("assets/daft-punk-something-about-us/daft-punk-something-about-us.mp3"),
    music_time(0.0f)
{
    music_slider.on_event = [&, music_is_playing = music.IsPlaying()](ui::slider::event slider_event) mutable
    {
        switch (slider_event)
        {
        case ui::slider::event::on_down:
            music_is_playing = music.IsPlaying();
            this->active_child = music_slider;
            this->music.Pause();
            break;
        case ui::slider::event::on_release:
            this->active_child = boost::none;
            this->music.Seek(music_slider.progress * this->music.GetTimeLength());
            if (music_is_playing)
                this->music.Resume();
            break;
        }
    };

    pause_play.on_event = [&](ui::button::event button_event)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            pause_play_color.set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
            pause_play_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down:
            pause_play_scale.set(std::nullopt, 0.9f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            pause_play_color.set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
            pause_play_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            break;
        }
    };

    pause_play.on_click = [&]()
    { 
        if (this->music.IsPlaying())
        {
            this->music.Pause();
            this->pause_play_img.set_image(play_png);

        }
        else
        {
            this->music.Seek(music_slider.progress * this->music.GetTimeLength());
            this->music.Resume();
            this->pause_play_img.set_image(pause_png);            
        }
    };

    const unsigned int music_length = static_cast<unsigned int>(music.GetTimeLength());
    const std::string music_length_str = std::format("0:00 / {}:{:02}", music_length / 60, music_length % 60);
    music_time_text.set_string(music_length_str);

    for (const auto& [id, rel_pos] : kee::key_ui_data)
    {
        kee::ui::base& key_holder = key_frame.add_child<kee::ui::base>(
            pos(pos::type::rel, rel_pos.x),
            pos(pos::type::rel, rel_pos.y),
            dims(
                dim(dim::type::aspect, id == KeyboardKey::KEY_SPACE ? 7.0f : 1.0f),
                dim(dim::type::rel, 0.25)
            ),
            kee::ui::common(true, std::nullopt, false)
        );

        keys.emplace(id, key_holder.add_child<editor_key>(*this, id));
    }

    /* TODO: for testing only */

    keys.at(KeyboardKey::KEY_W).get().push(editor_hit_object(0.0f, 16.0f));
    keys.at(KeyboardKey::KEY_Q).get().push(editor_hit_object(0.0f));
    keys.at(KeyboardKey::KEY_Q).get().push(editor_hit_object(4.0f));
    keys.at(KeyboardKey::KEY_Q).get().push(editor_hit_object(8.0f));
    keys.at(KeyboardKey::KEY_Q).get().push(editor_hit_object(12.0f));

    music.SetLooping(true);
    music.SetVolume(0.1f);
    music.Play();
}

float editor::get_beat() const
{
    return (music_time - music_start_offset) * music_bpm / 60.0f;
}

void editor::unselect()
{
    for (int prev_id : selected_key_ids)
    {
        keys.at(prev_id).get().set_opt_color(raylib::Color::White());
        keys.at(prev_id).get().is_selected = false;
    }

    selected_key_ids.clear();
}

void editor::select(int id)
{
    keys.at(id).get().set_opt_color(raylib::Color::Green());
    keys.at(id).get().is_selected = true;

    selected_key_ids.push_back(id);
}

void editor::handle_element_events()
{
    if (raylib::Keyboard::IsKeyPressed(KeyboardKey::KEY_SPACE))
    {
        if (raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_LEFT_CONTROL))
        {
            for (int prev_id : selected_key_ids)
                keys.at(prev_id).get().set_opt_color(raylib::Color::White());

            unselect();
        }
        else
            pause_play.on_click();
    }

    if (music.IsPlaying())
        return;

    mouse_wheel_move += raylib::Mouse::GetWheelMove();
    if (std::abs(mouse_wheel_move) < 1.0f)
        return;

    float new_beat = get_beat();
    float beat_steps = 0;

    if (mouse_wheel_move <= -1.0f)
    {
        beat_steps = -std::ceil(mouse_wheel_move);
        mouse_wheel_move += beat_steps;

        const float beat_floor = std::floorf(get_beat() / beat_step) * beat_step;
        new_beat = (std::abs(beat_floor - get_beat()) < editor::beat_lock_threshold) 
            ? beat_floor - beat_step * beat_steps 
            : beat_floor - beat_step * (beat_steps - 1);
    }
    else if (mouse_wheel_move >= 1.0f)
    {
        beat_steps = std::floor(mouse_wheel_move);
        mouse_wheel_move -= beat_steps;

        const float beat_ceil = std::ceilf(get_beat() / beat_step) * beat_step;
        new_beat = (std::abs(beat_ceil - get_beat()) < editor::beat_lock_threshold) 
            ? beat_ceil + beat_step * beat_steps 
            : beat_ceil + beat_step * (beat_steps - 1);
    }

    music_time = music_start_offset + new_beat * 60.0f / music_bpm;
}

void editor::update_element([[maybe_unused]] float dt)
{
    if (!music_slider.is_down())
    {
        if (music.IsPlaying())
        {
            music.Update();
            music_time = music.GetTimePlayed();
        }

        music_slider.progress = music_time / music.GetTimeLength();
    }
    else
        music_time = music_slider.progress * music.GetTimeLength();
    
    std::get<kee::dims>(pause_play_img.dimensions).w.val = pause_play_scale.get();
    std::get<kee::dims>(pause_play_img.dimensions).h.val = pause_play_scale.get();
    pause_play_img.set_opt_color(pause_play_color.get().to_color());

    const unsigned int music_length = static_cast<unsigned int>(music.GetTimeLength());
    const unsigned int music_time_int = static_cast<unsigned int>(music_time);
    const std::string music_time_str = std::format("{}:{:02} / {}:{:02}", music_time_int / 60, music_time_int % 60, music_length / 60, music_length % 60);
    music_time_text.set_string(music_time_str);
}

object_editor::object_editor(
    const kee::ui::base::required& reqs, 
    const kee::scene::editor& editor_scene,
    const std::unordered_map<int, std::reference_wrapper<editor_key>>& keys,
    const std::vector<int>& selected_key_ids
) :
    kee::ui::base(reqs,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.05f),
        dims(
            dim(dim::type::rel, 0.95f),
            dim(dim::type::rel, 0.1f)
        ),
        kee::ui::common(true, std::nullopt, false)
    ),
    editor_scene(editor_scene),
    keys(keys),
    selected_key_ids(selected_key_ids),
    beat_objects_frame(add_child<kee::ui::base>(
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.8f),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.2f)
        ),
        kee::ui::common(false, std::nullopt, false)
    )),
    beat_indicator(add_child<kee::ui::triangle>(
        raylib::Color::Red(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.1f),
        dims(
            dim(dim::type::aspect, 1.5),
            dim(dim::type::rel, 0.2f)
        ),
        raylib::Vector2(0, 0),
        raylib::Vector2(1, 0),
        raylib::Vector2(0.5, 1),
        kee::ui::common(true, 0, false)
    )),
    is_object_frame_down(false)
{ }

void object_editor::handle_element_events()
{
    const raylib::Vector2 mouse_pos = raylib::Mouse::GetPosition();
    const raylib::Rectangle beat_objects_raw_rect = beat_objects_frame.get_raw_rect();
    const bool is_mouse_on_rect =
        mouse_pos.x >= beat_objects_raw_rect.x && mouse_pos.x <= beat_objects_raw_rect.x + beat_objects_raw_rect.width &&
        mouse_pos.y >= beat_objects_raw_rect.y && mouse_pos.y <= beat_objects_raw_rect.y + beat_objects_raw_rect.height;

    if (is_object_frame_down)
    {
        if (raylib::Mouse::IsButtonReleased(MouseButton::MOUSE_BUTTON_LEFT))
        {
            /* TODO: on release */
            is_object_frame_down = false;
        }
        else
        {
            /* TODO: on drag */
        }
    }
    else if (is_mouse_on_rect && raylib::Mouse::IsButtonPressed(MouseButton::MOUSE_BUTTON_LEFT))
    {
        /* TODO: on down */
        is_object_frame_down = true;
    }
}

void object_editor::update_element([[maybe_unused]] float dt)
{
    object_rects.clear();

    for (const auto& [id, _] : kee::key_ui_data)
    for (const editor_hit_object& object : keys.at(id).get().get_hit_objects())
    {
        if (object.beat + object.duration < editor_scene.get_beat() - beat_delta)
            continue;

        if (object.beat > editor_scene.get_beat() + beat_delta)
            break;

        const float rel_x_beg = std::max(0.0f, (object.beat - (editor_scene.get_beat() - beat_delta)) / (2 * beat_delta));
        const float rel_x_end = std::min(1.0f, (object.beat + object.duration - (editor_scene.get_beat() - beat_delta)) / (2 * beat_delta));

        object_rects.push_back(beat_objects_frame.make_temp_child<kee::ui::rect>(
            raylib::Color(0, 0, 255, 30),
            pos(pos::type::rel, rel_x_beg),
            pos(pos::type::rel, 0),
            dims(
                dim(dim::type::rel, rel_x_end - rel_x_beg),
                dim(dim::type::rel, 1)
            ),
            kee::ui::rect_outline(kee::ui::rect_outline::type::rel_h, 0.15f, raylib::Color::Blue()), 
            kee::ui::rect_roundness(kee::ui::rect_roundness::type::rel_h, 0.5f, kee::ui::rect_roundness::size_effect::extend_w),
            kee::ui::common(false, std::nullopt, false)
        ));
    }
}

void object_editor::render_element_behind_children() const
{
    static constexpr float fade_percent = 0.05f;

    const float beat_start = std::ceil((editor_scene.get_beat() - beat_delta) / editor_scene.beat_step) * editor_scene.beat_step;
    for (float beat_render = beat_start; beat_render <= editor_scene.get_beat() + beat_delta; beat_render += editor_scene.beat_step)
    {
        const float render_rel_x = (beat_render - (editor_scene.get_beat() - beat_delta)) / (2 * beat_delta);

        float opacity = 1.0f;
        if (render_rel_x <= fade_percent)
            opacity = render_rel_x / fade_percent;
        else if (render_rel_x >= 1.0f - fade_percent)
            opacity = (1.0f - render_rel_x) / fade_percent;

        const bool is_whole_beat = std::floorf(beat_render) == beat_render;
        const float render_rel_w = is_whole_beat ? 0.003f : 0.001f;
        const float render_rel_h = is_whole_beat ? 0.2f : 0.1f;

        const kee::ui::rect beat_render_rect = make_temp_child<kee::ui::rect>(
            raylib::Color(255, 255, 255, static_cast<unsigned char>(255 * opacity)),
            pos(pos::type::rel, render_rel_x),
            pos(pos::type::rel, 0.2f),
            dims(
                dim(dim::type::rel, render_rel_w),
                dim(dim::type::rel, render_rel_h)
            ),
            std::nullopt,
            kee::ui::rect_roundness(kee::ui::rect_roundness::type::rel_w, 0.5f, std::nullopt),
            kee::ui::common(true, std::nullopt, false)
        );

        beat_render_rect.render();

        if (is_whole_beat)
        {
            const int whole_beat = static_cast<int>(std::floorf(beat_render));
            const kee::ui::text whole_beat_text = make_temp_child<kee::ui::text>(
                raylib::Color(255, 255, 255, static_cast<unsigned char>(255 * opacity)),
                pos(pos::type::rel, render_rel_x),
                pos(pos::type::rel, 0.55f),
                ui::text_size(ui::text_size::type::rel_h, 0.35f),
                assets.font_semi_bold, std::to_string(whole_beat), false,
                kee::ui::common(true, std::nullopt, false)
            );

            whole_beat_text.render();
        }
    }

    for (const kee::ui::rect& object_rect : object_rects)
    {
        const kee::ui::rect obj_circle_l = beat_objects_frame.make_temp_child<kee::ui::rect>(
            raylib::Color::Blue(),
            pos(pos::type::rel, object_rect.x.val),
            pos(pos::type::rel, 0.5f),
            dims(
                dim(dim::type::aspect, 1),
                dim(dim::type::rel, 0.3f)
            ),
            std::nullopt,
            kee::ui::rect_roundness(kee::ui::rect_roundness::type::rel_w, 0.5f, std::nullopt),
            kee::ui::common(true, std::nullopt, false)
        );

        const kee::ui::rect obj_circle_r = beat_objects_frame.make_temp_child<kee::ui::rect>(
            raylib::Color::Blue(),
            pos(pos::type::rel, object_rect.x.val + std::get<kee::dims>(object_rect.dimensions).w.val),
            pos(pos::type::rel, 0.5f),
            dims(
                dim(dim::type::aspect, 1),
                dim(dim::type::rel, 0.3f)
            ),
            std::nullopt,
            kee::ui::rect_roundness(kee::ui::rect_roundness::type::rel_w, 0.5f, std::nullopt),
            kee::ui::common(true, std::nullopt, false)
        );

        object_rect.render();
        obj_circle_l.render();
        obj_circle_r.render();
    }
}

editor_hit_object::editor_hit_object(float beat) :
    beat(beat),
    duration(0.0f)
{ }

editor_hit_object::editor_hit_object(float beat, float duration) :
    beat(beat),
    duration(duration)
{ }

editor_key::editor_key(const kee::ui::base::required& reqs, kee::scene::editor& editor_scene, int key_id) :
    kee::ui::button(reqs,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, kee::key_border_parent_h),
        kee::ui::common(true, std::nullopt, false)
    ),
    is_selected(false),
    editor_scene(editor_scene),
    frame(add_child<kee::ui::rect>(
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
    )),
    key_text(add_child<kee::ui::text>(
        std::nullopt,
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        ui::text_size(ui::text_size::type::rel_h, 0.5f),
        assets.font_light, std::string(), false,
        kee::ui::common(true, 0, false)
    )),
    key_id(key_id),
    is_control_clicked(false)
{
    on_event = [&](ui::button::event button_event)
    {
        if (button_event == ui::button::event::on_down)
        {
            if (raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_LEFT_CONTROL))
                this->is_control_clicked = true;
            else
                this->editor_scene.unselect();
        }
    };

    on_click = [&]()
    {
        this->editor_scene.select(this->key_id);
    };

    set_opt_color(raylib::Color::White());

    const std::string key_str = (key_id != KeyboardKey::KEY_SPACE)
        ? std::string(1, static_cast<char>(key_id))
        : "___";

    key_text.set_string(key_str);
}

const std::vector<editor_hit_object>& editor_key::get_hit_objects() const
{
    return hit_objects;
}

void editor_key::push(const editor_hit_object& object)
{
    if (!hit_objects.empty() && object.beat <= hit_objects.back().beat + hit_objects.back().duration)
        throw std::invalid_argument("A new hit object must be strictly after all other ones in its key!");

    hit_objects.push_back(object);
}

void editor_key::handle_element_events()
{
    kee::ui::button::handle_element_events();

    if (is_control_clicked && !raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_LEFT_CONTROL))
        is_control_clicked = false;
}

void editor_key::render_element_behind_children() const
{
    if (is_selected)
        render_hit_objects();
}

void editor_key::render_element_ahead_children() const
{
    if (!is_selected)
        render_hit_objects();
}

void editor_key::render_hit_objects() const
{
    for (const editor_hit_object& object : hit_objects)
    {
        if (object.beat + object.duration < editor_scene.get_beat() - editor::beat_lock_threshold)
            continue;

        if (object.beat > editor_scene.get_beat() + editor_scene.approach_beats)
            break;

        const float start_progress = std::max((object.beat - editor_scene.get_beat()) / (2 * editor_scene.approach_beats), 0.0f);
        const float end_progress = std::max((object.beat + object.duration - editor_scene.get_beat()) / (2 * editor_scene.approach_beats), 0.0f);
        const kee::ui::rect hit_obj_rect = make_temp_child<kee::ui::rect>(
            raylib::Color::Blank(),
            pos(pos::type::rel, 0.5),
            pos(pos::type::rel, 0.5),
            border(border::type::rel_h, start_progress),
            ui::rect_outline(ui::rect_outline::type::rel_h_parent, std::max(end_progress - start_progress, kee::key_border_width), raylib::Color::Red()),
            std::nullopt,
            kee::ui::common(true, std::nullopt, false)
        );

        hit_obj_rect.render();
    }
}

} // namespace scene
} // namespace kee