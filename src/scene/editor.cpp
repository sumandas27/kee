#include "kee/scene/editor.hpp"

#include <ranges>

namespace kee {
namespace scene {

editor_hit_object::editor_hit_object(int key, float duration) :
    key(key),
    duration(duration),
    is_selected(false)
{ }

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
    obj_editor(add_child<object_editor>(keys, selected_key_ids, *this)),
    approach_beats(2.0f),
    beat_step(0.25f),
    play_png("assets/img/play.png"),
    pause_png("assets/img/pause.png"),
    pause_play_color(add_transition<kee::color>(kee::color::white())),
    pause_play_scale(add_transition<float>(1.0f)),
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
        play_png,
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
        case ui::button::event::on_down_l:
            pause_play_scale.set(std::nullopt, 0.9f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            pause_play_color.set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
            pause_play_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            break;
        }
    };

    pause_play.on_click_l = [&]()
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

    keys.at(KeyboardKey::KEY_Q).get().hit_objects.emplace(0.0f, editor_hit_object(KeyboardKey::KEY_Q, 16.0f));
    keys.at(KeyboardKey::KEY_W).get().hit_objects.emplace(0.0f, editor_hit_object(KeyboardKey::KEY_W, 0.0f));
    keys.at(KeyboardKey::KEY_W).get().hit_objects.emplace(4.0f, editor_hit_object(KeyboardKey::KEY_W, 0.0f));
    keys.at(KeyboardKey::KEY_W).get().hit_objects.emplace(8.0f, editor_hit_object(KeyboardKey::KEY_W, 0.0f));
    keys.at(KeyboardKey::KEY_W).get().hit_objects.emplace(12.0f, editor_hit_object(KeyboardKey::KEY_W, 0.0f));

    music.SetLooping(true);
    music.SetVolume(0.1f);
    music.Play();
    music.Pause();

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
    obj_editor.reset_render_hit_objs();
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

    obj_editor.reset_render_hit_objs();
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
            pause_play.on_click_l();
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
    music.Update();

    if (!music_slider.is_down())
    {
        if (music.IsPlaying())
            music_time = music.GetTimePlayed();

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

hit_obj_render::hit_obj_render(hit_obj_ui&& render_ui, std::map<float, editor_hit_object>::iterator hit_obj_ref) :
    render_ui(std::move(render_ui)),
    hit_obj_ref(hit_obj_ref)
{ }

hit_obj_node::hit_obj_node(hit_obj_render& ref, std::map<float, editor_hit_object>::node_type node) :
    ref(ref),
    node(std::move(node))
{ }

void hit_obj_ui::select()
{
    set_opt_color(raylib::Color::DarkGreen());
    border.value().opt_color = raylib::Color::Green();
    circle_l.set_opt_color(raylib::Color::Green());
    circle_r.set_opt_color(raylib::Color::Green());
}

void hit_obj_ui::unselect()
{
    set_opt_color(raylib::Color::DarkBlue());
    border.value().opt_color = raylib::Color::Blue();
    circle_l.set_opt_color(raylib::Color::Blue());
    circle_r.set_opt_color(raylib::Color::Blue());
}

void hit_obj_ui::update(float beat, float duration, float curr_beat, float beat_width)
{
    x.val = (beat - curr_beat + beat_width) / (2 * beat_width);
    std::get<kee::dims>(dimensions).w.val = duration / (2 * beat_width);
}

new_hit_obj_data::new_hit_obj_data(int key, float rel_y, float click_beat, float current_beat) :
    key(key),
    rel_y(rel_y),
    click_beat(click_beat),
    current_beat(current_beat)
{ }

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
    selected_has_moved(false),
    new_hit_obj_from_editor(false)
{ }

void object_editor::reset_render_hit_objs()
{
    const std::vector<int>& keys_to_render = selected_key_ids.empty() ? editor::prio_to_key : selected_key_ids;

    obj_render_info.clear();
    for (std::size_t i = keys_to_render.size(); i-- > 0;)
    for (auto it = keys.at(keys_to_render[i]).get().hit_objects.begin(); it != keys.at(keys_to_render[i]).get().hit_objects.end(); it++)
    {
        auto& [beat, object] = *it;
        object.is_selected = false;

        const float rel_y = object_editor::hit_objs_rel_y + object_editor::hit_objs_rel_h * (i + 1) / (keys_to_render.size() + 1);
        hit_obj_ui render_ui = obj_renderer.make_temp_child<hit_obj_ui>(beat, object.duration, editor_scene.get_beat(), object_editor::beat_width, rel_y);
        obj_render_info.push_back(hit_obj_render(std::move(render_ui), it));
    }
}

void object_editor::attempt_add_hit_obj()
{
    const float new_beat = std::min(new_hit_object.value().click_beat, new_hit_object.value().current_beat);
    const float new_duration = std::max(new_hit_object.value().click_beat, new_hit_object.value().current_beat) - new_beat;

    std::map<float, editor_hit_object>& hit_objects = editor_scene.keys.at(new_hit_object.value().key).get().hit_objects;
    const auto next_it = hit_objects.lower_bound(new_beat);
    const bool is_new_invalid =
        (next_it != hit_objects.end() && new_beat + new_duration + editor::beat_lock_threshold >= next_it->first) ||
        (next_it != hit_objects.begin() && std::prev(next_it)->first + std::prev(next_it)->second.duration + editor::beat_lock_threshold >= new_beat);

    if (!is_new_invalid)
    {
        hit_objects.emplace(new_beat, editor_hit_object(new_hit_object.value().key, new_duration));
        reset_render_hit_objs();
    }

    new_hit_object.reset();
}

void object_editor::handle_element_events()
{
    if (raylib::Keyboard::IsKeyPressed(KeyboardKey::KEY_BACKSPACE) && !selected_is_active)
    {
        for (hit_obj_render& hit_obj : obj_render_info)
            if (hit_obj.hit_obj_ref->second.is_selected)
                editor_scene.keys.at(hit_obj.hit_obj_ref->second.key).get().hit_objects.erase(hit_obj.hit_obj_ref);

        reset_render_hit_objs();
    }

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
        const bool mouse_released_l = raylib::Mouse::IsButtonReleased(MouseButton::MOUSE_BUTTON_LEFT) && !new_hit_object.has_value();
        const bool mouse_released_r = raylib::Mouse::IsButtonReleased(MouseButton::MOUSE_BUTTON_RIGHT) && new_hit_object.has_value();
        if (!mouse_released_l && !mouse_released_r)
            return;

        if (mouse_released_l)
        {
            if (!selected_is_active)
            {
                const raylib::Rectangle selection_raw_rect = selection_rect.value().get_raw_rect();
                selection_rect.reset();

                if (selected_has_moved)
                {
                    selected_beat = std::numeric_limits<float>::max();
                    for (hit_obj_render& obj : obj_render_info)
                        if (selection_raw_rect.CheckCollision(obj.render_ui.get_extended_raw_rect()))
                        {
                            selected_beat = std::min(selected_beat, obj.hit_obj_ref->first);
                            obj.hit_obj_ref->second.is_selected = true;
                            obj.render_ui.select();
                        }

                    selected_reference_beat = selected_beat;
                }
            }
            else
            {
                const float beat_drag_width = mouse_beat - beat_drag_start.value();
                const float beat_drag_diff = std::round(beat_drag_width / editor_scene.beat_step) * editor_scene.beat_step;
                const float new_reference_beat = selected_reference_beat + beat_drag_diff;
                
                std::vector<hit_obj_node> hit_obj_nodes;
                for (hit_obj_render& obj_selected : obj_render_info)
                {
                    editor_hit_object& object = obj_selected.hit_obj_ref->second;
                    if (object.is_selected)
                    {
                        auto map_node = editor_scene.keys.at(object.key).get().hit_objects.extract(obj_selected.hit_obj_ref);
                        hit_obj_nodes.emplace_back(obj_selected, std::move(map_node));
                    }
                }

                bool is_move_valid = true;
                for (hit_obj_node& node : hit_obj_nodes)
                {
                    auto& [beat, object] = *node.ref.hit_obj_ref;
                    const float obj_beat_new = new_reference_beat + beat - selected_beat;

                    std::map<float, editor_hit_object>& hit_objects = editor_scene.keys.at(object.key).get().hit_objects;
                    auto next_it = hit_objects.lower_bound(obj_beat_new);
                    
                    if (next_it != hit_objects.end() && obj_beat_new + object.duration + editor::beat_lock_threshold >= next_it->first)
                        is_move_valid = false;
                    else if (next_it != hit_objects.begin() && std::prev(next_it)->first + std::prev(next_it)->second.duration + editor::beat_lock_threshold >= obj_beat_new)
                        is_move_valid = false;

                    if (!is_move_valid)
                        break;
                }

                for (hit_obj_node& node : hit_obj_nodes)
                {
                    auto& [beat, object] = *node.ref.hit_obj_ref;
                    const float obj_beat_new = new_reference_beat + beat - selected_beat;
                    const float obj_beat_inserted = is_move_valid ? obj_beat_new : beat;

                    node.node.key() = obj_beat_inserted;
                    node.ref.hit_obj_ref = editor_scene.keys.at(object.key).get().hit_objects.insert(std::move(node.node)).position;
                }

                selected_is_active = false;
                if (is_move_valid)
                {
                    selected_reference_beat = new_reference_beat;
                    selected_beat = selected_reference_beat;
                }
            }
        }
        else if (mouse_released_r)
        {
            attempt_add_hit_obj();        
            new_hit_obj_from_editor = false;
        }

        beat_drag_start.reset();
        selected_has_moved = false;
        editor_scene.active_child = boost::none;
    }
    else if (obj_renderer_rect.CheckCollision(mouse_pos))
    {
        if (!raylib::Mouse::IsButtonPressed(MouseButton::MOUSE_BUTTON_LEFT) && !raylib::Mouse::IsButtonPressed(MouseButton::MOUSE_BUTTON_RIGHT))
            return;

        mouse_pos_start = mouse_pos;
        beat_drag_start = mouse_beat;
        editor_scene.active_child = *this;

        if (raylib::Mouse::IsButtonPressed(MouseButton::MOUSE_BUTTON_LEFT))
        {
            /**
             * If two hit objects intersect with mouse position on-click, already selected objects take
             * priority. Then the top-rightmost one (top-most prioritized over right-most) gets selected 
             * since that is how hit objects are ordered when rendered.
             */
            static const auto click_priority = [](const hit_obj_render& l, const hit_obj_render& r)
            {
                if (l.hit_obj_ref->second.is_selected != r.hit_obj_ref->second.is_selected)
                    return r.hit_obj_ref->second.is_selected;

                const raylib::Rectangle l_rect = l.render_ui.get_extended_raw_rect();
                const raylib::Rectangle r_rect = r.render_ui.get_extended_raw_rect();

                return (l_rect.y != r_rect.y)
                    ? l_rect.y > r_rect.y
                    : l_rect.x < r_rect.x;
            };

            auto obj_rects_clicked = obj_render_info | std::views::filter(
                [&mouse_pos](const hit_obj_render& obj){ return obj.render_ui.get_extended_raw_rect().CheckCollision(mouse_pos); }
            );

            auto obj_selected_it = std::ranges::max_element(obj_rects_clicked, click_priority);
            if (obj_selected_it == std::ranges::end(obj_rects_clicked))
            {
                for (hit_obj_render& obj : obj_render_info)
                    if (obj.hit_obj_ref->second.is_selected)
                    {
                        obj.hit_obj_ref->second.is_selected = false;
                        obj.render_ui.unselect();
                    }

                selection_rect.emplace(obj_renderer.make_temp_child<kee::ui::rect>(
                    raylib::Color(255, 255, 255, 50),
                    pos(pos::type::rel, 0),
                    pos(pos::type::rel, 0),
                    dims(
                        dim(dim::type::rel, 0),
                        dim(dim::type::rel, 1)
                    ),
                    std::nullopt, std::nullopt,
                    kee::ui::common(false, std::nullopt, false)
                ));
            }
            else
            {
                selected_beat = obj_selected_it->hit_obj_ref->first;
                selected_reference_beat = selected_beat;
                selected_is_active = true;

                if (obj_selected_it->hit_obj_ref->second.is_selected)
                    return;

                if (!raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_LEFT_CONTROL))
                    for (hit_obj_render& obj : obj_render_info)
                        if (obj.hit_obj_ref->second.is_selected)
                        {
                            obj.hit_obj_ref->second.is_selected = false;
                            obj.render_ui.unselect();
                        }

                obj_selected_it->hit_obj_ref->second.is_selected = true;
                obj_selected_it->render_ui.select();
            }
        }
        else if (raylib::Mouse::IsButtonPressed(MouseButton::MOUSE_BUTTON_RIGHT))
        {
            const float mouse_rel_y = (mouse_pos.y - obj_renderer_rect.y) / obj_renderer_rect.height;

            const std::vector<int>& keys_to_render = selected_key_ids.empty() ? editor::prio_to_key : selected_key_ids;
            const float key_rel_y_range = object_editor::hit_objs_rel_h / (keys_to_render.size() + 1);
            const float keys_rel_y_beg = object_editor::hit_objs_rel_y + object_editor::hit_objs_rel_h * 1 / (keys_to_render.size() + 1) - key_rel_y_range / 2;
            const float keys_rel_y_end = object_editor::hit_objs_rel_y + object_editor::hit_objs_rel_h * keys_to_render.size() / (keys_to_render.size() + 1) + key_rel_y_range / 2;

            std::size_t selected_key_idx;
            if (mouse_rel_y < keys_rel_y_beg)
                selected_key_idx = 0;
            else if (mouse_rel_y > keys_rel_y_end)
                selected_key_idx = keys_to_render.size() - 1;
            else
                selected_key_idx = static_cast<std::size_t>((mouse_rel_y - keys_rel_y_beg) / key_rel_y_range);

            const float start_beat = std::round(mouse_beat / editor_scene.beat_step) * editor_scene.beat_step;
            const float rel_y = object_editor::hit_objs_rel_y + object_editor::hit_objs_rel_h * (selected_key_idx + 1) / (keys_to_render.size() + 1);
            
            new_hit_object = new_hit_obj_data(keys_to_render[selected_key_idx], rel_y, start_beat, start_beat);
            new_hit_obj_from_editor = true;
        }
    }
}

void object_editor::update_element([[maybe_unused]] float dt)
{
    if (selection_rect.has_value())
    {
        const float selection_start_beat = std::min(mouse_beat, beat_drag_start.value());
        const float selection_end_beat = std::max(mouse_beat, beat_drag_start.value());

        selection_rect.value().x.val = (selection_start_beat - (editor_scene.get_beat() - beat_width)) / (2 * beat_width);
        std::get<kee::dims>(selection_rect.value().dimensions).w.val = (selection_end_beat - selection_start_beat) / (2 * beat_width);
    }
    else if (new_hit_object.has_value())
    {
        const float current_beat = new_hit_obj_from_editor 
            ? std::round(mouse_beat / editor_scene.beat_step) * editor_scene.beat_step
            : editor_scene.get_beat();
            
        new_hit_object.value().current_beat = current_beat;
    }

    float beat_drag_diff = 0.0f;
    if (beat_drag_start.has_value())
    {
        if (!selected_has_moved && raylib::Mouse::GetPosition() != mouse_pos_start)
        {
            if (selected_is_active && std::abs(std::fmod(selected_reference_beat, editor_scene.beat_step)) > editor_scene.beat_lock_threshold)
            {
                selected_reference_beat = std::round(selected_reference_beat / editor_scene.beat_step) * editor_scene.beat_step;
                beat_drag_start = selected_reference_beat;
            }

            selected_has_moved = true;
        }

        if (selected_is_active)
        {
            const float beat_drag_width = mouse_beat - beat_drag_start.value();
            beat_drag_diff = std::round(beat_drag_width / editor_scene.beat_step) * editor_scene.beat_step;
        }
    }

    for (hit_obj_render& hit_obj : obj_render_info)
    {
        auto& [beat, object] = *hit_obj.hit_obj_ref;
        const float beat_update = object.is_selected ? selected_reference_beat + beat_drag_diff + beat - selected_beat : beat;

        hit_obj.render_ui.update(beat_update, object.duration, editor_scene.get_beat(), beat_width);
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
        if (object_rect.get_raw_rect().CheckCollision(obj_renderer.get_raw_rect()) && !obj_ref->second.is_selected)
            object_rect.render();

    for (const auto& [object_rect, obj_ref] : obj_render_info)
        if (object_rect.get_raw_rect().CheckCollision(obj_renderer.get_raw_rect()) && obj_ref->second.is_selected)
            object_rect.render();

    if (selection_rect.has_value())
        selection_rect.value().render();
    else if (new_hit_object.has_value())
    {
        const float beg_beat = std::min(new_hit_object.value().click_beat, new_hit_object.value().current_beat);
        const float end_beat = std::max(new_hit_object.value().click_beat, new_hit_object.value().current_beat);

        hit_obj_ui new_hit_obj_render = obj_renderer.make_temp_child<hit_obj_ui>(beg_beat, end_beat - beg_beat, editor_scene.get_beat(), beat_width, new_hit_object.value().rel_y);
        new_hit_obj_render.select();
        new_hit_obj_render.render();
    }
}

void object_editor::render_element_ahead_children() const
{
    const std::vector<int>& keys_to_render = selected_key_ids.empty() ? editor::prio_to_key : selected_key_ids;
    if (keys_to_render.size() <= 6)
        for (std::size_t i = 0; i < keys_to_render.size(); i++)
        {
            const float rel_y = object_editor::hit_objs_rel_y + object_editor::hit_objs_rel_h * (i + 1) / (keys_to_render.size() + 1);
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
        switch (button_event)
        {
        case ui::button::event::on_down_l:
            if (raylib::Keyboard::IsKeyDown(KeyboardKey::KEY_LEFT_CONTROL))
                this->is_control_clicked = true;
            break;
        case ui::button::event::on_down_r: {
            this->editor_scene.unselect();
            this->editor_scene.select(this->key_id);

            static constexpr float new_hit_obj_rel_y = object_editor::hit_objs_rel_y + object_editor::hit_objs_rel_h / 2;
            this->editor_scene.obj_editor.new_hit_object = new_hit_obj_data(this->key_id, new_hit_obj_rel_y, this->editor_scene.get_beat(), this->editor_scene.get_beat());
            break;
        }
        case ui::button::event::on_leave:
            if (this->editor_scene.obj_editor.new_hit_object.has_value())
            {
                this->editor_scene.unselect();
                this->editor_scene.obj_editor.new_hit_object.reset();
            }
            break;
        default:
            break;
        }
    };

    on_click_l = [&]()
    {
        if (!this->is_control_clicked)
            this->editor_scene.unselect();
        this->editor_scene.select(this->key_id);
    };

    on_click_r = [&]()
    {
        this->editor_scene.obj_editor.attempt_add_hit_obj();

        this->editor_scene.unselect();
        this->editor_scene.obj_editor.new_hit_object.reset();
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
    for (const auto& [beat, object] : hit_objects)
    {
        if (beat + object.duration < editor_scene.get_beat() - editor::beat_lock_threshold)
            continue;

        if (beat > editor_scene.get_beat() + editor_scene.approach_beats)
            break;

        const float start_progress = std::max((beat - editor_scene.get_beat()) / (2 * editor_scene.approach_beats), 0.0f);
        const float end_progress = std::max((beat + object.duration - editor_scene.get_beat()) / (2 * editor_scene.approach_beats), 0.0f);
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