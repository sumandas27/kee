#include "kee/scene/editor.hpp"

#include <ranges>

namespace kee {
namespace scene {

const std::vector<int> editor::prio_to_key = []
{
    std::vector<int> res;
    for (const kee::key_pos_data& data : kee::key_ui_data)
        res.push_back(data.raylib_key);
    return res;
}();

const std::unordered_map<int, int> editor::key_to_prio = []
{
    std::unordered_map<int, int> res;
    for (int i = 0; i < kee::key_ui_data.size(); i++)
        res.emplace(kee::key_ui_data[i].raylib_key, i);
    return res;
}();

editor::editor(const kee::scene::window& window, kee::global_assets& assets) :
    kee::scene::base(window, assets),
    approach_beats(2.0f),
    beat_step(0.25f),
    play_png("assets/img/play.png"),
    pause_png("assets/img/pause.png"),
    pause_play_color(add_transition<kee::color>(kee::color::white())),
    pause_play_scale(add_transition<float>(1.0f)),
    obj_editor(add_child<object_editor>(keys, selected_key_ids, *this)),
    music_slider(add_child<kee::ui::slider>(
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.22f),
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
        pos(pos::type::rel, 0.55f),
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

    keys.at(KeyboardKey::KEY_W).get().hit_objects.push_back(editor_hit_object(0.0f, 16.0f));
    keys.at(KeyboardKey::KEY_Q).get().hit_objects.push_back(editor_hit_object(0.1f));
    keys.at(KeyboardKey::KEY_Q).get().hit_objects.push_back(editor_hit_object(4.0f));
    keys.at(KeyboardKey::KEY_Q).get().hit_objects.push_back(editor_hit_object(8.0f));
    keys.at(KeyboardKey::KEY_Q).get().hit_objects.push_back(editor_hit_object(12.0f));

    music.SetLooping(true);
    music.SetVolume(0.1f);
    music.Play();

    unselect();
}

bool editor::is_music_playing() const
{
    return music.IsPlaying();
}

float editor::get_beat() const
{
    return (music_time - music_start_offset) * music_bpm / 60.0f;
}

void editor::set_beat(float new_beat)
{
    const float music_time_raw = music_start_offset + new_beat * 60.0f / music_bpm;
    music_time = std::clamp(music_time_raw, 0.0f, music.GetTimeLength());
}

void editor::unselect()
{
    for (int prev_id : selected_key_ids)
    {
        keys.at(prev_id).get().set_opt_color(raylib::Color::White());
        keys.at(prev_id).get().is_selected = false;
    }

    selected_key_ids.clear();
    obj_editor.obj_render_info.clear();

    for (std::size_t i = editor::prio_to_key.size(); i-- > 0;)
    for (editor_hit_object& object : keys.at(editor::prio_to_key[i]).get().hit_objects)
    {
        object.is_selected = false;

        const float rel_y = 0.1f + 0.6f * (i + 1) / (editor::prio_to_key.size() + 1);
        hit_obj_ui render_ui = obj_editor.obj_renderer.make_temp_child<hit_obj_ui>(object.beat, object.duration, get_beat(), object_editor::beat_width, rel_y);
        obj_editor.obj_render_info.push_back(hit_obj_render(std::move(render_ui), object));
    }
}

void editor::select(int id)
{
    keys.at(id).get().set_opt_color(raylib::Color::Green());
    keys.at(id).get().is_selected = true;

    selected_key_ids.insert(
        std::lower_bound(selected_key_ids.begin(), selected_key_ids.end(), id,
            [](int l, int r) { return editor::key_to_prio.at(l) <= editor::key_to_prio.at(r); }
        ),
    id);

    obj_editor.obj_render_info.clear();
    for (std::size_t i = selected_key_ids.size(); i-- > 0;)
    for (editor_hit_object& object : keys.at(selected_key_ids[i]).get().hit_objects)
    {
        object.is_selected = false;

        const float rel_y = 0.1f + 0.6f * (i + 1) / (selected_key_ids.size() + 1);
        hit_obj_ui render_ui = obj_editor.obj_renderer.make_temp_child<hit_obj_ui>(object.beat, object.duration, get_beat(), object_editor::beat_width, rel_y);
        obj_editor.obj_render_info.push_back(hit_obj_render(std::move(render_ui), object));
    }
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

    if (is_music_playing())
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

    set_beat(new_beat);
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

editor_hit_object::editor_hit_object(float beat) :
    beat(beat),
    duration(0.0f),
    is_selected(false)
{ }

editor_hit_object::editor_hit_object(float beat, float duration) :
    beat(beat),
    duration(duration),
    is_selected(false)
{ }

hit_obj_ui::hit_obj_ui(const kee::ui::base::required& reqs, float beat, float duration, float curr_beat, float beat_width, float rel_y) :
    kee::ui::rect(reqs,
        raylib::Color::DarkBlue(),
        pos(pos::type::rel, (beat - curr_beat + beat_width) / (2 * beat_width)),
        pos(pos::type::rel, rel_y - rel_h / 2),
        dims(
            dim(dim::type::rel, duration / (2 * beat_width)),
            dim(dim::type::rel, rel_h)
        ),
        kee::ui::rect_outline(kee::ui::rect_outline::type::rel_h, 0.15f, raylib::Color::Blue()), 
        kee::ui::rect_roundness(kee::ui::rect_roundness::type::rel_h, 0.5f, kee::ui::rect_roundness::size_effect::extend_w),
        kee::ui::common(false, std::nullopt, false)
    ),
    circle_l(add_child<kee::ui::rect>(
        raylib::Color::Blue(),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.4f)
        ),
        std::nullopt,
        kee::ui::rect_roundness(kee::ui::rect_roundness::type::rel_w, 0.5f, std::nullopt),
        kee::ui::common(true, std::nullopt, false)
    )),
    circle_r(add_child<kee::ui::rect>(
        raylib::Color::Blue(),
        pos(pos::type::rel, 1),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.4f)
        ),
        std::nullopt,
        kee::ui::rect_roundness(kee::ui::rect_roundness::type::rel_w, 0.5f, std::nullopt),
        kee::ui::common(true, std::nullopt, false)
    ))
{ }

hit_obj_render::hit_obj_render(hit_obj_ui&& render_ui, editor_hit_object& hit_obj_ref) :
    render_ui(std::move(render_ui)),
    hit_obj_ref(hit_obj_ref)
{ }

void hit_obj_ui::update(float beat, float duration, float curr_beat, float beat_width)
{
    x.val = (beat - curr_beat + beat_width) / (2 * beat_width);
    std::get<kee::dims>(dimensions).w.val = duration / (2 * beat_width);
}

object_editor::object_editor(
    const kee::ui::base::required& reqs,
    const std::unordered_map<int, std::reference_wrapper<editor_key>>& keys,
    const std::vector<int>& selected_key_ids,
    kee::scene::editor& editor_scene
) :
    kee::ui::rect(reqs,
        raylib::Color(15, 15, 15, 255),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.1f),
        dims(
            dim(dim::type::rel, 1.0f),
            dim(dim::type::rel, 0.2f)
        ),
        std::nullopt, std::nullopt,
        kee::ui::common(true, std::nullopt, false)
    ),
    obj_renderer(add_child<kee::ui::base>(
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::rel, 0.75f),
            dim(dim::type::rel, 1)
        ),
        kee::ui::common(true, std::nullopt, false)
    )),
    keys(keys),
    selected_key_ids(selected_key_ids),
    editor_scene(editor_scene),
    beat_hover_l(add_child<kee::ui::base>(
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 0.01f),
            dim(dim::type::rel, 1)
        ),
        kee::ui::common(false, std::nullopt, false)
    )),
    beat_hover_r(add_child<kee::ui::base>(
        pos(pos::type::rel, 0.99f),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 0.01f),
            dim(dim::type::rel, 1)
        ),
        kee::ui::common(false, std::nullopt, false)
    )),
    rect_l(add_child<kee::ui::rect>(
        raylib::Color(30, 30, 30),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 0.125f),
            dim(dim::type::rel, 1)
        ),
        std::nullopt, std::nullopt,
        kee::ui::common(false, std::nullopt, false)
    )),
    rect_r(add_child<kee::ui::rect>(
        raylib::Color(30, 30, 30),
        pos(pos::type::rel, 0.875f),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 0.125f),
            dim(dim::type::rel, 1)
        ),
        std::nullopt, std::nullopt,
        kee::ui::common(false, std::nullopt, false)
    )),
    beat_indicator(add_child<kee::ui::triangle>(
        raylib::Color::Red(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.05f),
        dims(
            dim(dim::type::aspect, 1.5),
            dim(dim::type::rel, 0.1f)
        ),
        raylib::Vector2(0, 0),
        raylib::Vector2(1, 0),
        raylib::Vector2(0.5, 1),
        kee::ui::common(true, 0, false)
    )),
    selected_is_active(false),
    selected_has_moved(false)
{ }

void object_editor::handle_element_events()
{
    if (editor_scene.is_music_playing())
        return;

    const raylib::Vector2 mouse_pos = raylib::Mouse::GetPosition();
    const raylib::Rectangle obj_renderer_rect = obj_renderer.get_raw_rect();

    const float mouse_in_rect_percent = (mouse_pos.x - obj_renderer_rect.x) / obj_renderer_rect.width;
    const float mouse_beat_diff = (mouse_in_rect_percent - 0.5f) * beat_width * 2;
    mouse_beat = editor_scene.get_beat() + mouse_beat_diff;

    if (beat_hover_l.get_raw_rect().CheckCollision(mouse_pos))
        beat_drag_multiplier = -1;
    else if (beat_hover_r.get_raw_rect().CheckCollision(mouse_pos))
        beat_drag_multiplier = 1;
    else
        beat_drag_multiplier = 0;

    if (beat_drag_start.has_value())
    {
        if (raylib::Mouse::IsButtonReleased(MouseButton::MOUSE_BUTTON_LEFT))
        {
            editor_scene.active_child = boost::none;

            beat_drag_start.reset();
            if (selected_is_active)
            {
                /* TODO: code release */
                selected_is_active = false;
            }
            else
            {
                /* TODO: release selection box */
            }
        }
        else
        {
            if (selected_is_active)
            {
                if (!selected_has_moved && mouse_pos != mouse_pos_start)
                {
                    if (std::abs(std::fmod(selected_reference_beat, editor_scene.beat_step)) > editor_scene.beat_lock_threshold)
                    {
                        selected_reference_beat = std::round(selected_reference_beat / editor_scene.beat_step) * editor_scene.beat_step;
                        beat_drag_start = selected_reference_beat;
                    }

                    selected_has_moved = true;
                }
            }
            else
            {
                /* TODO: edit selection box */
            }
        }
    }
    else if (obj_renderer_rect.CheckCollision(mouse_pos) && raylib::Mouse::IsButtonPressed(MouseButton::MOUSE_BUTTON_LEFT))
    {
        /**
         * If two hit objects intersect with mouse position on-click, the top-rightmost one (top-most prioritized 
         * over right-most) gets selected since that is how the hit objects are ordered when rendered.
         */
        static const auto click_priority = [](const hit_obj_render& l, const hit_obj_render& r)
        {
            const raylib::Rectangle l_rect = l.render_ui.get_extended_raw_rect();
            const raylib::Rectangle r_rect = r.render_ui.get_extended_raw_rect();

            return (l_rect.y != r_rect.y)
                ? l_rect.y > r_rect.y
                : l_rect.x < r_rect.x;
        };

        auto obj_rects_clicked = obj_render_info | std::views::filter(
            [&mouse_pos](const hit_obj_render& obj){ 
                return !obj.hit_obj_ref.get().is_selected && obj.render_ui.get_extended_raw_rect().CheckCollision(mouse_pos); 
            }
        );

        auto obj_selected_it = std::ranges::max_element(obj_rects_clicked, click_priority);
        if (obj_selected_it == std::ranges::end(obj_rects_clicked))
        {
            std::println("DRAWING SELECTION BOX");
        }
        else
        {
            for (hit_obj_render& obj_unselected : obj_render_info)
            {
                obj_unselected.hit_obj_ref.get().is_selected = false;
                obj_unselected.render_ui.set_opt_color(raylib::Color::DarkBlue());
                obj_unselected.render_ui.border.value().opt_color = raylib::Color::Blue();
                obj_unselected.render_ui.circle_l.set_opt_color(raylib::Color::Blue());
                obj_unselected.render_ui.circle_r.set_opt_color(raylib::Color::Blue());
            }

            mouse_pos_start = mouse_pos;
            selected_reference_beat = obj_selected_it->hit_obj_ref.get().beat;
            selected_is_active = true;

            obj_selected_it->hit_obj_ref.get().is_selected = true;
            obj_selected_it->render_ui.set_opt_color(raylib::Color::DarkGreen());
            obj_selected_it->render_ui.border.value().opt_color = raylib::Color::Green();
            obj_selected_it->render_ui.circle_l.set_opt_color(raylib::Color::Green());
            obj_selected_it->render_ui.circle_r.set_opt_color(raylib::Color::Green());
        }

        beat_drag_start = mouse_beat;
        editor_scene.active_child = *this;
    }    
}

void object_editor::update_element([[maybe_unused]] float dt)
{
    /* TODO: group `beat_drag_start` + `reference_beat` */
    float beat_drag_diff;
    if (beat_drag_start.has_value())
    {
        const float beat_drag_width = mouse_beat - beat_drag_start.value();
        beat_drag_diff = std::round(beat_drag_width / editor_scene.beat_step) * editor_scene.beat_step;
    }
    else
        beat_drag_diff = 0;

    for (hit_obj_render& hit_obj : obj_render_info)
    {
        const float beat = hit_obj.hit_obj_ref.get().is_selected
            ? selected_reference_beat + beat_drag_diff
            : hit_obj.hit_obj_ref.get().beat;

        hit_obj.render_ui.update(beat, hit_obj.hit_obj_ref.get().duration, editor_scene.get_beat(), beat_width);
    }

    const float beat_delta = beat_drag_multiplier * beat_drag_speed * dt;
    editor_scene.set_beat(editor_scene.get_beat() + beat_delta);
}

void object_editor::render_element_behind_children() const
{
    kee::ui::rect::render_element_behind_children();

    const float beat_start = std::ceil((editor_scene.get_beat() - beat_width) / editor_scene.beat_step) * editor_scene.beat_step;
    for (float beat_render = beat_start - 1.0f; beat_render <= editor_scene.get_beat() + beat_width + 1.0f; beat_render += editor_scene.beat_step)
    {
        const float render_rel_x = (beat_render - (editor_scene.get_beat() - beat_width)) / (2 * beat_width);

        const bool is_whole_beat = std::floorf(beat_render) == beat_render;
        const float render_rel_w = is_whole_beat ? 0.005f : 0.003f;
        const float render_rel_h = is_whole_beat ? 0.1f : 0.05f;

        const kee::ui::rect beat_render_rect = obj_renderer.make_temp_child<kee::ui::rect>(
            raylib::Color::White(),
            pos(pos::type::rel, render_rel_x),
            pos(pos::type::rel, 0.75f),
            dims(
                dim(dim::type::rel, render_rel_w),
                dim(dim::type::rel, render_rel_h)
            ),
            std::nullopt,
            kee::ui::rect_roundness(kee::ui::rect_roundness::type::rel_w, 0.5f, std::nullopt),
            kee::ui::common(true, std::nullopt, false)
        );

        if (beat_render_rect.get_raw_rect().CheckCollision(obj_renderer.get_raw_rect()))
            beat_render_rect.render();

        if (is_whole_beat)
        {
            const int whole_beat = static_cast<int>(std::floorf(beat_render));
            const kee::ui::text whole_beat_text = obj_renderer.make_temp_child<kee::ui::text>(
                raylib::Color::White(),
                pos(pos::type::rel, render_rel_x),
                pos(pos::type::rel, 0.9f),
                ui::text_size(ui::text_size::type::rel_h, 0.15f),
                assets.font_semi_bold, std::to_string(whole_beat), true,
                kee::ui::common(true, std::nullopt, false)
            );

            if (whole_beat_text.get_raw_rect().CheckCollision(obj_renderer.get_raw_rect()))
                whole_beat_text.render();
        }
    }

    for (const auto& [object_rect, obj_ref] : obj_render_info)
        if (object_rect.get_raw_rect().CheckCollision(obj_renderer.get_raw_rect()) && !obj_ref.get().is_selected)
            object_rect.render();

    for (const auto& [object_rect, obj_ref] : obj_render_info)
        if (object_rect.get_raw_rect().CheckCollision(obj_renderer.get_raw_rect()) && obj_ref.get().is_selected)
            object_rect.render();
}

void object_editor::render_element_ahead_children() const
{
    const std::vector<int>& keys_to_render = selected_key_ids.empty() ? editor::prio_to_key : selected_key_ids;
    if (keys_to_render.size() <= 6)
        for (std::size_t i = 0; i < keys_to_render.size(); i++)
        {
            const float rel_y = 0.1f + 0.6f * (i + 1) / (keys_to_render.size() + 1);
            const std::string key_str = (keys_to_render[i] != KeyboardKey::KEY_SPACE)
                ? std::string(1, static_cast<char>(keys_to_render[i]))
                : "__";

            const kee::ui::text key_to_render_text = make_temp_child<kee::ui::text>(
                raylib::Color::White(),
                pos(pos::type::rel, 0.115f),
                pos(pos::type::rel, rel_y),
                ui::text_size(ui::text_size::type::rel_h, 0.1f),
                assets.font_semi_bold, key_str, false,
                kee::ui::common(true, std::nullopt, false)
            );

            key_to_render_text.render();
        }
}

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
        if (button_event == ui::button::event::on_down && raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_LEFT_CONTROL))
            this->is_control_clicked = true;
    };

    on_click = [&]()
    {
        if (!this->is_control_clicked)
            this->editor_scene.unselect();
        this->editor_scene.select(this->key_id);
    };

    set_opt_color(raylib::Color::White());

    const std::string key_str = (key_id != KeyboardKey::KEY_SPACE)
        ? std::string(1, static_cast<char>(key_id))
        : "___";

    key_text.set_string(key_str);
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