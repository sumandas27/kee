#include "kee/scene/editor.hpp"

#include <ranges>

namespace kee {
namespace scene {

editor_hit_object::editor_hit_object(int key, float duration) :
    key(key),
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
        false,
        kee::ui::rect_outline(kee::ui::rect_outline::type::rel_h, 0.15f, raylib::Color::Blue()), 
        kee::ui::rect_roundness(kee::ui::rect_roundness::type::rel_h, 0.5f, kee::ui::rect_roundness::size_effect::extend_w)
    ),
    circle_l(add_child<kee::ui::rect>(std::nullopt,
        raylib::Color::Blue(),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.4f)
        ),
        true,
        std::nullopt,
        kee::ui::rect_roundness(kee::ui::rect_roundness::type::rel_w, 0.5f, std::nullopt)
    )),
    circle_r(add_child<kee::ui::rect>(std::nullopt,
        raylib::Color::Blue(),
        pos(pos::type::rel, 1),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.4f)
        ),
        true,
        std::nullopt,
        kee::ui::rect_roundness(kee::ui::rect_roundness::type::rel_w, 0.5f, std::nullopt)
    ))
{ }

hit_obj_render::hit_obj_render(hit_obj_ui&& render_ui, std::map<float, editor_hit_object>::iterator hit_obj_ref) :
    render_ui(std::move(render_ui)),
    hit_obj_ref(hit_obj_ref)
{ }

hit_obj_update_return::hit_obj_update_return(float beat, float duration) :
    beat(beat),
    duration(duration)
{ }

hit_obj_node::hit_obj_node(int key, float old_beat, float old_duration, hit_obj_update_return obj_new, std::map<float, editor_hit_object>::iterator& hit_obj_ref, std::map<float, editor_hit_object>::node_type node) :
    key(key),
    old_beat(old_beat),
    old_duration(old_duration),
    obj_new(obj_new),
    invalid_to_populate(hit_obj_ref),
    node(std::move(node))
{ }

void hit_obj_ui::select()
{
    set_opt_color(raylib::Color::DarkGreen());
    border.value().opt_color = raylib::Color::Green();
    circle_l.ref.set_opt_color(raylib::Color::Green());
    circle_r.ref.set_opt_color(raylib::Color::Green());
}

void hit_obj_ui::unselect()
{
    set_opt_color(raylib::Color::DarkBlue());
    border.value().opt_color = raylib::Color::Blue();
    circle_l.ref.set_opt_color(raylib::Color::Blue());
    circle_r.ref.set_opt_color(raylib::Color::Blue());
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

drag_selection::drag_selection(std::map<float, editor_hit_object>::iterator hold_obj_ref) :
    hold_obj_ref(hold_obj_ref)
{ }

editor_key::editor_key(const kee::ui::base::required& reqs, kee::scene::editor& editor_scene, int key_id) :
    kee::ui::button(reqs,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, kee::key_border_parent_h),
        true
    ),
    frame(add_child<kee::ui::rect>(-1,
        raylib::Color::Blank(),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        true,
        ui::rect_outline(ui::rect_outline::type::rel_h, kee::key_border_width, std::nullopt),
        std::nullopt
    )),
    key_text(add_child<kee::ui::text>(-1,
        std::nullopt,
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        ui::text_size(ui::text_size::type::rel_h, 0.5f),
        true, assets.font_light, std::string(), false
    )),
    is_selected(false),
    editor_scene(editor_scene),
    key_id(key_id)
{
    on_event = [&](ui::button::event button_event, bool ctrl_modifier)
    {
        switch (button_event)
        {
        case ui::button::event::on_down_l:
            if (!ctrl_modifier)
                this->editor_scene.unselect();

            this->editor_scene.select(this->key_id);
            break;
        case ui::button::event::on_down_r: {
            this->editor_scene.unselect();
            this->editor_scene.select(this->key_id);

            static constexpr float new_hit_obj_rel_y = object_editor::hit_objs_rel_y + object_editor::hit_objs_rel_h / 2;
            this->editor_scene.obj_editor.ref.new_hit_object = new_hit_obj_data(this->key_id, new_hit_obj_rel_y, this->editor_scene.get_beat(), this->editor_scene.get_beat());
            break;
        }
        case ui::button::event::on_leave:
            if (this->editor_scene.obj_editor.ref.new_hit_object.has_value())
            {
                this->editor_scene.unselect();
                this->editor_scene.obj_editor.ref.new_hit_object.reset();
            }
            break;
        default:
            break;
        }
    };

    on_click_r = [&]([[maybe_unused]] bool ctrl_modifier)
    {
        this->editor_scene.obj_editor.ref.attempt_add_hit_obj();

        this->editor_scene.unselect();
        this->editor_scene.obj_editor.ref.new_hit_object.reset();
    };

    set_opt_color(raylib::Color::White());

    const std::string key_str = (key_id != KeyboardKey::KEY_SPACE)
        ? std::string(1, static_cast<char>(key_id))
        : "___";

    key_text.ref.set_string(key_str);
}

void editor_key::update_element([[maybe_unused]] float dt)
{
    hit_obj_rects.clear();
    for (const auto& [beat, object] : hit_objects)
    {
        if (beat + object.duration < editor_scene.get_beat() - editor::beat_lock_threshold)
            continue;

        if (beat > editor_scene.get_beat() + editor_scene.approach_beats)
            break;

        const float start_progress = std::max((beat - editor_scene.get_beat()) / (2 * editor_scene.approach_beats), 0.0f);
        const float end_progress = std::max((beat + object.duration - editor_scene.get_beat()) / (2 * editor_scene.approach_beats), 0.0f);
        hit_obj_rects.push_back(make_temp_child<kee::ui::rect>(
            raylib::Color::Blank(),
            pos(pos::type::rel, 0.5),
            pos(pos::type::rel, 0.5),
            border(border::type::rel_h, start_progress),
            true,
            ui::rect_outline(ui::rect_outline::type::rel_h_parent, std::max(end_progress - start_progress, kee::key_border_width), raylib::Color::Red()),
            std::nullopt
        ));
    }
}

void editor_key::render_element() const
{
    for (const kee::ui::rect& hit_obj_rect : hit_obj_rects)
        hit_obj_rect.render();
}

object_editor::object_editor(
    const kee::ui::base::required& reqs,
    const std::vector<int>& selected_key_ids,
    std::unordered_map<int, kee::ui::handle<editor_key>>& keys,
    kee::scene::editor& editor_scene
) :
    kee::ui::rect(reqs,
        raylib::Color(15, 15, 15, 255),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 1.0f),
            dim(dim::type::rel, 0.2f)
        ),
        false, std::nullopt, std::nullopt
    ),
    obj_renderer(add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 0.8f),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    selected_key_ids(selected_key_ids),
    keys(keys),
    editor_scene(editor_scene),
    beat_hover_l(add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 0.01f),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    beat_hover_r(add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.99f),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 0.01f),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    settings_rect(add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(30, 30, 30),
        pos(pos::type::rel, 0.8f),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 0.2f),
            dim(dim::type::rel, 1)
        ),
        false, std::nullopt, std::nullopt
    )),
    key_label_rect(settings_rect.ref.add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(35, 35, 35),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 0.075f),
            dim(dim::type::rel, 1)
        ),
        false, std::nullopt, std::nullopt
    )),
    beat_indicator(add_child<kee::ui::triangle>(std::nullopt,
        raylib::Color::Red(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.05f),
        dims(
            dim(dim::type::aspect, 1.5),
            dim(dim::type::rel, 0.1f)
        ),
        true,
        raylib::Vector2(0, 0),
        raylib::Vector2(1, 0),
        raylib::Vector2(0.5, 1)
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
    for (auto it = keys.at(keys_to_render[i]).ref.hit_objects.begin(); it != keys.at(keys_to_render[i]).ref.hit_objects.end(); it++)
    {
        auto& [beat, object] = *it;
        object.is_selected = false;

        const float rel_y = object_editor::hit_objs_rel_y + object_editor::hit_objs_rel_h * (i + 1) / (keys_to_render.size() + 1);
        hit_obj_ui render_ui = make_temp_child<hit_obj_ui>(beat, object.duration, editor_scene.get_beat(), object_editor::beat_width, rel_y);
        obj_render_info.push_back(hit_obj_render(std::move(render_ui), it));
    }

    key_labels.clear();
    if (keys_to_render.size() <= 6)
        for (std::size_t i = 0; i < keys_to_render.size(); i++)
        {
            const float rel_y = object_editor::hit_objs_rel_y + object_editor::hit_objs_rel_h * (i + 1) / (keys_to_render.size() + 1);
            const std::string key_str = (keys_to_render[i] != KeyboardKey::KEY_SPACE)
                ? std::string(1, static_cast<char>(keys_to_render[i]))
                : "__";

            key_labels.push_back(key_label_rect.ref.add_child<kee::ui::text>(std::nullopt,
                raylib::Color::White(),
                pos(pos::type::rel, 0.5f),
                pos(pos::type::rel, rel_y),
                ui::text_size(ui::text_size::type::rel_h, 0.1f),
                true, assets.font_semi_bold, key_str, false
            ));
        }
}

bool object_editor::delete_selected_hit_objs()
{
    if (selected_is_active)
        return false;

    for (hit_obj_render& hit_obj : obj_render_info)
        if (hit_obj.hit_obj_ref->second.is_selected)
            keys.at(hit_obj.hit_obj_ref->second.key).ref.hit_objects.erase(hit_obj.hit_obj_ref);

    reset_render_hit_objs();
    return true;
}

void object_editor::attempt_add_hit_obj()
{
    const float new_beat = std::min(new_hit_object.value().click_beat, new_hit_object.value().current_beat);
    const float new_duration = std::max(new_hit_object.value().click_beat, new_hit_object.value().current_beat) - new_beat;

    std::map<float, editor_hit_object>& hit_objects = keys.at(new_hit_object.value().key).ref.hit_objects;
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

void object_editor::on_element_mouse_move(const raylib::Vector2& mouse_pos, [[maybe_unused]] bool ctrl_modifier)
{
    if (editor_scene.is_music_playing())
        return;

    const float mouse_in_rect_percent = (mouse_pos.x - get_raw_rect().x) / get_raw_rect().width;
    const float mouse_beat_diff = (mouse_in_rect_percent - 0.5f) * beat_width * 2;
    mouse_beat = editor_scene.get_beat() + mouse_beat_diff;

    if (beat_hover_l.ref.get_raw_rect().CheckCollision(mouse_pos))
        beat_drag_multiplier = -1;
    else if (beat_hover_r.ref.get_raw_rect().CheckCollision(mouse_pos))
        beat_drag_multiplier = 1;
    else
        beat_drag_multiplier = 0;

    if (selection_rect.has_value())
    {
        const float selection_start_beat = std::min(mouse_beat, beat_drag_start.value());
        const float selection_end_beat = std::max(mouse_beat, beat_drag_start.value());

        selection_rect.value().x.val = (selection_start_beat - (editor_scene.get_beat() - beat_width)) / (2 * beat_width);
        std::get<kee::dims>(selection_rect.value().dimensions).w.val = (selection_end_beat - selection_start_beat) / (2 * beat_width);
    }
    else if (new_hit_object.has_value())
    { 
        float current_beat;
        if (new_hit_obj_from_editor)
        {
            current_beat = mouse_beat;
            if (editor_scene.is_beat_snap_enabled())
                current_beat = std::round(current_beat * editor_scene.get_ticks_per_beat()) / editor_scene.get_ticks_per_beat();
        }
        else
            current_beat = editor_scene.get_beat();

        new_hit_object.value().current_beat = current_beat;
    }

    if (beat_drag_start.has_value() && !selected_has_moved && mouse_pos != mouse_pos_start)
    {
        if (selected_is_active && std::abs(std::fmod(selected_reference_beat, 1.0f / editor_scene.get_ticks_per_beat())) > editor_scene.beat_lock_threshold && editor_scene.is_beat_snap_enabled())
            selected_reference_beat = std::round(selected_reference_beat * editor_scene.get_ticks_per_beat()) / editor_scene.get_ticks_per_beat();

        selected_has_moved = true;
    }
}

bool object_editor::on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, bool ctrl_modifier)
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

    const raylib::Rectangle obj_renderer_rect = obj_renderer.ref.get_raw_rect();
    if (!obj_renderer_rect.CheckCollision(mouse_pos) || editor_scene.is_music_playing())
        return false;

    mouse_pos_start = mouse_pos;
    beat_drag_start = mouse_beat;

    if (is_mouse_l)
    {
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

            selection_rect.emplace(make_temp_child<kee::ui::rect>(
                raylib::Color(255, 255, 255, 50),
                pos(pos::type::rel, 0),
                pos(pos::type::rel, 0),
                dims(
                    dim(dim::type::rel, 0),
                    dim(dim::type::rel, 1)
                ),
                false, std::nullopt, std::nullopt
            ));

            return true;
        }
        
        bool is_drag_possible = true;
        if (!obj_selected_it->hit_obj_ref->second.is_selected)
        {
            if (ctrl_modifier)
                is_drag_possible = false;
            else
                for (hit_obj_render& obj : obj_render_info)
                    if (obj.hit_obj_ref->second.is_selected)
                    {
                        obj.hit_obj_ref->second.is_selected = false;
                        obj.render_ui.unselect();
                    }

            obj_selected_it->hit_obj_ref->second.is_selected = true;
            obj_selected_it->render_ui.select();
        }
        else
        {
            is_drag_possible = std::none_of(obj_render_info.begin(), obj_render_info.end(), 
                [&](const hit_obj_render& obj) 
                {
                    const bool distinct_from_selected = &obj != &(*obj_selected_it);
                    return distinct_from_selected && obj.hit_obj_ref->second.is_selected;
                }
            );
        }

        if (is_drag_possible && obj_selected_it->hit_obj_ref->second.duration > editor::beat_lock_threshold)
        {
            const bool collide_with_l_dot = obj_selected_it->render_ui.circle_l.ref.get_raw_rect().CheckCollision(mouse_pos);
            const bool collide_with_r_dot = obj_selected_it->render_ui.circle_r.ref.get_raw_rect().CheckCollision(mouse_pos);

            if (collide_with_l_dot || collide_with_r_dot)
            {
                const float selected_obj_midpoint_x = obj_selected_it->render_ui.get_raw_rect().x + obj_selected_it->render_ui.get_raw_rect().width / 2;

                hit_obj_drag_selection = drag_selection(obj_selected_it->hit_obj_ref);
                hit_obj_drag_selection.value().is_left = (collide_with_l_dot && collide_with_r_dot)
                    ? mouse_pos.x < selected_obj_midpoint_x
                    : collide_with_l_dot;
            }
        }

        selected_beat = obj_selected_it->hit_obj_ref->first;
        selected_is_active = true;
        selected_reference_beat = (hit_obj_drag_selection.has_value() && !hit_obj_drag_selection.value().is_left) 
            ? obj_selected_it->hit_obj_ref->first + obj_selected_it->hit_obj_ref->second.duration
            : obj_selected_it->hit_obj_ref->first;
    }
    else
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

        const float rel_y = object_editor::hit_objs_rel_y + object_editor::hit_objs_rel_h * (selected_key_idx + 1) / (keys_to_render.size() + 1);     
        float start_beat = mouse_beat;
        if (editor_scene.is_beat_snap_enabled())
            start_beat = std::round(start_beat * editor_scene.get_ticks_per_beat()) / editor_scene.get_ticks_per_beat();

        new_hit_object = new_hit_obj_data(keys_to_render[selected_key_idx], rel_y, start_beat, start_beat);
        new_hit_obj_from_editor = true;
    }

    return true;
}

bool object_editor::on_element_mouse_up([[maybe_unused]] const raylib::Vector2& mouse_pos, bool is_mouse_l, [[maybe_unused]] bool ctrl_modifier)
{
    if (editor_scene.is_music_playing() || !beat_drag_start.has_value() || is_mouse_l == new_hit_object.has_value())
        return false;

    if (!is_mouse_l)
    {
        attempt_add_hit_obj();        
        new_hit_obj_from_editor = false;

        beat_drag_start.reset();
        selected_has_moved = false;
        return true;
    }

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
        const float beat_drag_diff = get_beat_drag_diff();

        std::vector<hit_obj_node> hit_obj_nodes;
        for (hit_obj_render& obj_selected : obj_render_info)
        {
            editor_hit_object& object = obj_selected.hit_obj_ref->second;
            if (object.is_selected)
            {
                const int key = obj_selected.hit_obj_ref->second.key;
                const float old_beat = obj_selected.hit_obj_ref->first;
                const float old_duration = obj_selected.hit_obj_ref->second.duration;
                const hit_obj_update_return new_obj = hit_obj_update_info(obj_selected, beat_drag_diff);

                auto map_node = keys.at(object.key).ref.hit_objects.extract(obj_selected.hit_obj_ref);
                hit_obj_nodes.emplace_back(key, old_beat, old_duration, new_obj, obj_selected.hit_obj_ref, std::move(map_node));
            }
        }

        bool is_move_valid = true;
        for (hit_obj_node& node : hit_obj_nodes)
        {
            std::map<float, editor_hit_object>& hit_objects = keys.at(node.key).ref.hit_objects;
            auto next_it = hit_objects.lower_bound(node.obj_new.beat);
            
            if (next_it != hit_objects.end() && node.obj_new.beat + node.obj_new.duration + editor::beat_lock_threshold >= next_it->first)
                is_move_valid = false;
            else if (next_it != hit_objects.begin() && std::prev(next_it)->first + std::prev(next_it)->second.duration + editor::beat_lock_threshold >= node.obj_new.beat)
                is_move_valid = false;

            if (!is_move_valid)
                break;
        }

        selected_reference_beat = std::numeric_limits<float>::max();
        for (hit_obj_node& node : hit_obj_nodes)
        {
            const hit_obj_update_return obj_insert = is_move_valid ? node.obj_new : hit_obj_update_return(node.old_beat, node.old_duration);
            if (selected_reference_beat > obj_insert.beat)
                selected_reference_beat = obj_insert.beat;

            node.node.key() = obj_insert.beat;
            node.node.mapped().duration = obj_insert.duration;
            node.invalid_to_populate = keys.at(node.key).ref.hit_objects.insert(std::move(node.node)).position;
        }

        selected_is_active = false;
        selected_beat = selected_reference_beat;
        hit_obj_drag_selection.reset();
    }

    beat_drag_start.reset();
    selected_has_moved = false;
    return true;
}

void object_editor::update_element([[maybe_unused]] float dt)
{
    const float beat_drag_diff = get_beat_drag_diff();
    for (hit_obj_render& hit_obj : obj_render_info)
    {
        hit_obj_update_return update = hit_obj_update_info(hit_obj, beat_drag_diff);
        hit_obj.render_ui.update(update.beat, update.duration, editor_scene.get_beat(), beat_width);
    }

    const int beat_start_tick = static_cast<int>(std::floor(editor_scene.get_beat() - beat_width) * editor_scene.get_ticks_per_beat());
    const int beat_end_tick = static_cast<int>(std::ceil(editor_scene.get_beat() + beat_width) * editor_scene.get_ticks_per_beat());
    
    beat_render_rects.clear();
    whole_beat_texts.clear();
    for (int beat_tick = beat_start_tick; beat_tick <= beat_end_tick; beat_tick++)
    {
        const float beat_render = static_cast<float>(beat_tick) / editor_scene.get_ticks_per_beat();
        const float render_rel_x = (beat_render - (editor_scene.get_beat() - beat_width)) / (2 * beat_width);

        const bool is_whole_beat = beat_tick % editor_scene.get_ticks_per_beat() == 0;
        const float render_rel_w = is_whole_beat ? 0.005f : 0.003f;
        const float render_rel_h = is_whole_beat ? 0.1f : 0.05f;

        beat_render_rects.push_back(make_temp_child<kee::ui::rect>(
            raylib::Color::White(),
            pos(pos::type::rel, render_rel_x),
            pos(pos::type::rel, 0.75f),
            dims(
                dim(dim::type::rel, render_rel_w),
                dim(dim::type::rel, render_rel_h)
            ),
            true,
            std::nullopt,
            kee::ui::rect_roundness(kee::ui::rect_roundness::type::rel_w, 0.5f, std::nullopt)
        ));

        if (is_whole_beat)
        {
            const int whole_beat = beat_tick / editor_scene.get_ticks_per_beat();
            whole_beat_texts.push_back(make_temp_child<kee::ui::text>(
                raylib::Color::White(),
                pos(pos::type::rel, render_rel_x),
                pos(pos::type::rel, 0.9f),
                ui::text_size(ui::text_size::type::rel_h, 0.15f),
                true, assets.font_semi_bold, std::to_string(whole_beat), true
            ));
        }
    }

    if (new_hit_object.has_value())
    {
        const float beg_beat = std::min(new_hit_object.value().click_beat, new_hit_object.value().current_beat);
        const float end_beat = std::max(new_hit_object.value().click_beat, new_hit_object.value().current_beat);
        
        new_hit_obj_render.emplace(make_temp_child<hit_obj_ui>(beg_beat, end_beat - beg_beat, editor_scene.get_beat(), beat_width, new_hit_object.value().rel_y));
        new_hit_obj_render.value().select();
    }

    const float beat_delta = beat_drag_multiplier * beat_drag_speed * dt;
    editor_scene.set_beat(editor_scene.get_beat() + beat_delta);
}

void object_editor::render_element() const
{
    kee::ui::rect::render_element();

    for (const kee::ui::rect& beat_render_rect : beat_render_rects)
        if (beat_render_rect.get_raw_rect().CheckCollision(obj_renderer.ref.get_raw_rect()))
            beat_render_rect.render();

    for (const kee::ui::text& whole_beat_text : whole_beat_texts)
        if (whole_beat_text.get_raw_rect().CheckCollision(obj_renderer.ref.get_raw_rect()))
            whole_beat_text.render();

    for (const auto& [object_rect, obj_ref] : obj_render_info)
        if (object_rect.get_raw_rect().CheckCollision(obj_renderer.ref.get_raw_rect()) && !obj_ref->second.is_selected)
            object_rect.render();

    for (const auto& [object_rect, obj_ref] : obj_render_info)
        if (object_rect.get_raw_rect().CheckCollision(obj_renderer.ref.get_raw_rect()) && obj_ref->second.is_selected)
            object_rect.render();

    if (selection_rect.has_value())
        selection_rect.value().render();
    else if (new_hit_object.has_value())
        new_hit_obj_render.value().render();
}

float object_editor::get_beat_drag_diff() const
{
    if (beat_drag_start.has_value() && selected_is_active)
    {
        float beat_drag_diff = mouse_beat - beat_drag_start.value();
        if (editor_scene.is_beat_snap_enabled())
            beat_drag_diff = std::round(beat_drag_diff * editor_scene.get_ticks_per_beat()) / editor_scene.get_ticks_per_beat();
    
        return beat_drag_diff;
    }
    else
        return 0.0f;
}

hit_obj_update_return object_editor::hit_obj_update_info(const hit_obj_render& hit_obj, float beat_drag_diff) const
{
    auto& [beat, object] = *hit_obj.hit_obj_ref;

    float update_beat = beat;
    float update_duration = object.duration;
    if (object.is_selected)
    {
        if (!hit_obj_drag_selection.has_value())
            update_beat = selected_reference_beat + beat_drag_diff + beat - selected_beat;
        else if (hit_obj_drag_selection.value().is_left)
        {
            float max_hit_obj_beat = std::floor((beat + object.duration) * editor_scene.get_ticks_per_beat()) / editor_scene.get_ticks_per_beat();
            if (std::abs(beat + object.duration - max_hit_obj_beat) <= editor::beat_lock_threshold)
                max_hit_obj_beat -= 1.0f / editor_scene.get_ticks_per_beat();

            update_beat = std::min(selected_reference_beat + beat_drag_diff, max_hit_obj_beat);
            update_duration = (beat + object.duration) - update_beat;
        }
        else
        {
            float min_hit_obj_beat = std::ceil(beat * editor_scene.get_ticks_per_beat()) / editor_scene.get_ticks_per_beat();
            if (std::abs(beat - min_hit_obj_beat) <= editor::beat_lock_threshold)
                min_hit_obj_beat += 1.0f / editor_scene.get_ticks_per_beat();

            const float update_end_beat = std::max(selected_reference_beat + beat_drag_diff, min_hit_obj_beat);
            update_duration = update_end_beat - beat;
        }
    }

    return hit_obj_update_return(update_beat, update_duration);
}

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

/* TODO: create smaller rect in inspector to store ui elements so everything is aligned */

editor::editor(const kee::scene::window& window, kee::global_assets& assets) :
    kee::scene::base(window, assets),
    approach_beats(2.0f),
    music_start_offset(0.5f),
    obj_editor(add_child<object_editor>(std::nullopt, selected_key_ids, keys, *this)),
    music_bpm(100.0f),
    pause_png("assets/img/pause.png"),
    arrow_png("assets/img/arrow.png"),
    tick_freq_idx(3),
    pause_play_color(add_transition<kee::color>(kee::color::white())),
    pause_play_scale(add_transition<float>(1.0f)),
    beat_snap_button_color(add_transition<kee::color>(kee::color::white())),
    beat_snap_button_outline(add_transition<float>(0.6f)),
    tick_l_button_color(add_transition<kee::color>(kee::color::white())),
    tick_r_button_color(add_transition<kee::color>(kee::color::white())),
    tick_l_button_scale(add_transition<float>(1.0f)),
    tick_r_button_scale(add_transition<float>(1.0f)),
    tick_curr_rect_x(add_transition<float>(static_cast<float>(tick_freq_idx * 2 + 1) / (tick_freq_count * 2))),
    inspector_rect(add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(20, 20, 20),
        pos(pos::type::rel, 0.8f),
        pos(pos::type::rel, 0.2f),
        dims(
            dim(dim::type::rel, 0.2f),
            dim(dim::type::rel, 0.8f)
        ),
        false, std::nullopt, std::nullopt
    )),
    beat_snap_button(inspector_rect.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.05f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.02f)
        ),
        true
    )),
    beat_snap_button_rect(beat_snap_button.ref.add_child<kee::ui::rect>(std::nullopt,
        raylib::Color::Blank(),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        false,
        kee::ui::rect_outline(kee::ui::rect_outline::type::rel_h, 0.6f, raylib::Color::White()),
        std::nullopt
    )),
    beat_snap_text(inspector_rect.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.35f),
        pos(pos::type::rel, 0.05f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        true, assets.font_semi_bold, "BEAT SNAP", false
    )),
    tick_text(inspector_rect.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.125f),
        ui::text_size(ui::text_size::type::rel_h, 0.06f),
        true, assets.font_semi_bold, "1 / " + std::to_string(get_ticks_per_beat()), false
    )),
    tick_l_button(inspector_rect.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.2f),
        pos(pos::type::rel, 0.125f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.035f)
        ),
        true
    )),
    tick_l_img(tick_l_button.ref.add_child<kee::ui::image>(std::nullopt,
        arrow_png, raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        true, false, false, 0.0f
    )),
    tick_r_button(inspector_rect.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.8f),
        pos(pos::type::rel, 0.125f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.035f)
        ),
        true
    )),
    tick_r_img(tick_r_button.ref.add_child<kee::ui::image>(std::nullopt,
        arrow_png, raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        true, true, false, 0.0f
    )),
    tick_frame(inspector_rect.ref.add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(30, 30, 30, 255),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.18f),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.03f)
        ),
        false, std::nullopt, std::nullopt
    )),
    tick_curr_rect(tick_frame.ref.add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(40, 40, 40, 255),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::rel, 1.0f / tick_freq_count),
            dim(dim::type::rel, 1)
        ),
        true, std::nullopt, std::nullopt
    )),
    playback_speed_text(inspector_rect.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.295f),
        pos(pos::type::rel, 0.255f),
        ui::text_size(ui::text_size::type::rel_h, 0.025f),
        true, assets.font_semi_bold, "PLAYBACK SPEED:", false
    )),
    playback_dropdown(inspector_rect.ref.add_child<kee::ui::dropdown>(std::nullopt,
        pos(pos::type::rel, 0.75f),
        pos(pos::type::rel, 0.255f),
        dims(
            dim(dim::type::rel, 0.3f),
            dim(dim::type::rel, 0.03f)
        ),
        true, 
        std::vector<std::string>(
            std::from_range,
            playback_speeds | std::views::transform([](float playback_speed){
                return std::format("{:.2f}x", playback_speed);
            })
        ),
        std::ranges::distance(playback_speeds.begin(), std::ranges::find(playback_speeds, 1.0f))
    )),
    music_slider(add_child<kee::ui::slider>(std::nullopt,
        pos(pos::type::rel, 0.01f),
        pos(pos::type::rel, 0.22f),
        dims(
            dim(dim::type::rel, 0.78f),
            dim(dim::type::rel, 0.005f)
        ),
        false
    )),
    pause_play(music_slider.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 40),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::abs, 60)
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
    music_time_text(music_slider.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::end, 0),
        pos(pos::type::beg, 30),
        ui::text_size(ui::text_size::type::abs, 80),
        false, assets.font_regular, std::string(), false
    )),
    key_border(add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.4f),
        pos(pos::type::rel, 0.6f),
        dims(
            dim(dim::type::rel, 0.7f),
            dim(dim::type::rel, 0.5f)
        ),
        true
    )),
    key_frame(key_border.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 10),
            dim(dim::type::aspect, 4)
        ),
        true
    )),
    mouse_wheel_move(0.0f),
    is_beat_snap(true),
    music("assets/daft-punk-something-about-us/daft-punk-something-about-us.mp3"),
    music_time(0.0f)
{
    for (std::size_t i = 0; i < tick_freq_count; i++)
    {
        tick_frame_texts.push_back(tick_frame.ref.add_child<kee::ui::text>(std::nullopt,
            raylib::Color::White(),
            pos(pos::type::rel, static_cast<float>(i * 2 + 1) / (tick_freq_count * 2)),
            pos(pos::type::rel, 0.5f),
            ui::text_size(ui::text_size::type::rel_h, 0.7f),
            true, assets.font_regular, std::to_string(tick_freqs[i]), false
        ));

        tick_frame_buttons.push_back(tick_frame.ref.add_child<kee::ui::button>(std::nullopt,
            pos(pos::type::rel, static_cast<float>(i * 2 + 1) / (tick_freq_count * 2)),
            pos(pos::type::rel, 0.5f),
            dims(
                dim(dim::type::rel, 1.0f / tick_freq_count),
                dim(dim::type::rel, 1)
            ),
            true
        ));

        tick_frame_buttons.back().ref.on_event = [&, idx = i](ui::button::event button_event, [[maybe_unused]] bool ctrl_modifier)
        {
            switch (button_event)
            {
            case ui::button::event::on_hot:
                this->tick_frame_text_colors[idx].get().set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
                break;
            case ui::button::event::on_leave:
                this->tick_frame_text_colors[idx].get().set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
                break;
            default:
                break;
            }
        };

        tick_frame_buttons.back().ref.on_click_l = [&, idx = i]([[maybe_unused]] bool ctrl_modifier)
        {
            this->set_tick_freq_idx(idx);
        };

        tick_frame_text_colors.push_back(add_transition<kee::color>(kee::color::white()));
    }

    tick_l_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] bool ctrl_modifier)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->tick_l_button_color.set(std::nullopt, kee::color::dark_orange(), 0.555f, kee::transition_type::exp);
            this->tick_l_button_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            this->tick_l_button_scale.set(std::nullopt, 0.9f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->tick_l_button_color.set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
            this->tick_l_button_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    tick_l_button.ref.on_click_l = [&]([[maybe_unused]] bool ctrl_modifier)
    {
        if (this->tick_freq_idx != 0)
            this->set_tick_freq_idx(this->tick_freq_idx - 1);
    };

    tick_r_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] bool ctrl_modifier)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->tick_r_button_color.set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
            this->tick_r_button_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            this->tick_r_button_scale.set(std::nullopt, 0.9f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->tick_r_button_color.set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
            this->tick_r_button_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    tick_r_button.ref.on_click_l = [&]([[maybe_unused]] bool ctrl_modifier)
    {
        if (this->tick_freq_idx != editor::tick_freq_count - 1)
            this->set_tick_freq_idx(this->tick_freq_idx + 1);
    };

    beat_snap_button.ref.on_click_l = [&]([[maybe_unused]] bool ctrl_modifier)
    {
        if (this->is_beat_snap)
        {
            this->beat_snap_button_outline.set(std::nullopt, 0.2f, 0.25f, kee::transition_type::exp);
            this->is_beat_snap = false;
        }
        else
        {
            this->beat_snap_button_outline.set(std::nullopt, 0.6f, 0.25f, kee::transition_type::exp);
            this->is_beat_snap = true;
        }
    };

    beat_snap_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] bool ctrl_modifier)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->beat_snap_button_color.set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->beat_snap_button_color.set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

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

    pause_play.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] bool ctrl_modifier)
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

    pause_play.ref.on_click_l = [&]([[maybe_unused]] bool ctrl_modifier)
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

    for (const auto& [id, rel_pos] : kee::key_ui_data)
    {
        key_holders.push_back(key_frame.ref.add_child<kee::ui::base>(std::nullopt,
            pos(pos::type::rel, rel_pos.x),
            pos(pos::type::rel, rel_pos.y),
            dims(
                dim(dim::type::aspect, id == KeyboardKey::KEY_SPACE ? 7.0f : 1.0f),
                dim(dim::type::rel, 0.25)
            ),
            true
        ));

        keys.emplace(id, key_holders.back().ref.add_child<editor_key>(std::nullopt, *this, id));
    }

    /* TODO: for testing only */

    keys.at(KeyboardKey::KEY_Q).ref.hit_objects.emplace(0.2f, editor_hit_object(KeyboardKey::KEY_Q, 16.0f));
    keys.at(KeyboardKey::KEY_W).ref.hit_objects.emplace(0.0f, editor_hit_object(KeyboardKey::KEY_W, 0.0f));
    keys.at(KeyboardKey::KEY_W).ref.hit_objects.emplace(4.0f, editor_hit_object(KeyboardKey::KEY_W, 0.0f));
    keys.at(KeyboardKey::KEY_W).ref.hit_objects.emplace(8.0f, editor_hit_object(KeyboardKey::KEY_W, 0.0f));
    keys.at(KeyboardKey::KEY_W).ref.hit_objects.emplace(12.0f, editor_hit_object(KeyboardKey::KEY_W, 0.0f));

    music.SetLooping(true);
    music.SetVolume(0.1f);
    music.Play();
    music.Pause();

    unselect();
}

int editor::get_ticks_per_beat() const
{
    return tick_freqs[tick_freq_idx];
}

bool editor::is_music_playing() const
{
    return music.IsPlaying();
}

bool editor::is_beat_snap_enabled() const
{
    return is_beat_snap;
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
        keys.at(prev_id).ref.set_opt_color(raylib::Color::White());
        keys.at(prev_id).ref.is_selected = false;
        keys.at(prev_id).ref.frame.change_z_order(-1);
        keys.at(prev_id).ref.key_text.change_z_order(-1);
    }

    selected_key_ids.clear();
    obj_editor.ref.reset_render_hit_objs();
}

void editor::select(int id)
{
    keys.at(id).ref.set_opt_color(raylib::Color::Green());
    keys.at(id).ref.is_selected = true;
    keys.at(id).ref.frame.change_z_order(0);
    keys.at(id).ref.key_text.change_z_order(0);

    selected_key_ids.insert(
        std::lower_bound(selected_key_ids.begin(), selected_key_ids.end(), id,
            [](int l, int r) { return editor::key_to_prio.at(l) <= editor::key_to_prio.at(r); }
        ),
    id);

    obj_editor.ref.reset_render_hit_objs();
}

bool editor::on_element_key_down(int keycode, bool ctrl_modifier)
{
    switch (keycode)
    {
    case KeyboardKey::KEY_SPACE:
        if (ctrl_modifier)
        {
            for (int prev_id : selected_key_ids)
                keys.at(prev_id).ref.set_opt_color(raylib::Color::White());

            unselect();
        }
        else
            pause_play.ref.on_click_l(ctrl_modifier);

        return true;
    case KeyboardKey::KEY_BACKSPACE:
        return obj_editor.ref.delete_selected_hit_objs();
    default:
        return false;
    }
}

bool editor::on_element_mouse_scroll(float mouse_scroll)
{
    if (is_music_playing())
        return false;

    mouse_wheel_move += mouse_scroll;
    if (std::abs(mouse_wheel_move) < 1.0f)
        return false;

    float new_beat = get_beat();
    float beat_steps = 0;

    if (mouse_wheel_move <= -1.0f)
    {
        beat_steps = -std::ceil(mouse_wheel_move);
        mouse_wheel_move += beat_steps;

        const float beat_floor = std::floorf(get_beat() * get_ticks_per_beat()) / get_ticks_per_beat();
        new_beat = (std::abs(beat_floor - get_beat()) < editor::beat_lock_threshold) 
            ? beat_floor - (1.0f / get_ticks_per_beat()) * beat_steps 
            : beat_floor - (1.0f / get_ticks_per_beat()) * (beat_steps - 1);
    }
    else if (mouse_wheel_move >= 1.0f)
    {
        beat_steps = std::floor(mouse_wheel_move);
        mouse_wheel_move -= beat_steps;

        const float beat_ceil = std::ceilf(get_beat() * get_ticks_per_beat()) / get_ticks_per_beat();
        new_beat = (std::abs(beat_ceil - get_beat()) < editor::beat_lock_threshold) 
            ? beat_ceil + (1.0f / get_ticks_per_beat()) * beat_steps 
            : beat_ceil + (1.0f / get_ticks_per_beat()) * (beat_steps - 1);
    }

    set_beat(new_beat);
    return true;
}

void editor::update_element([[maybe_unused]] float dt)
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

    beat_snap_button_rect.ref.border.value().opt_color.value() = beat_snap_button_color.get().to_color();
    beat_snap_button_rect.ref.border.value().val = beat_snap_button_outline.get();

    std::get<kee::dims>(tick_l_img.ref.dimensions).w.val = tick_l_button_scale.get();
    std::get<kee::dims>(tick_l_img.ref.dimensions).h.val = tick_l_button_scale.get();
    std::get<kee::dims>(tick_r_img.ref.dimensions).w.val = tick_r_button_scale.get();
    std::get<kee::dims>(tick_r_img.ref.dimensions).h.val = tick_r_button_scale.get();

    for (std::size_t i = 0; i < editor::tick_freq_count; i++)
        tick_frame_texts[i].ref.set_opt_color(tick_frame_text_colors[i].get().get().to_color());

    const raylib::Color tick_l_img_color = (tick_freq_idx != 0) ? tick_l_button_color.get().to_color() : raylib::Color::Blank();
    const raylib::Color tick_r_img_color = (tick_freq_idx != tick_freq_count - 1) ? tick_r_button_color.get().to_color() : raylib::Color::Blank();
    tick_l_img.ref.set_opt_color(tick_l_img_color);
    tick_r_img.ref.set_opt_color(tick_r_img_color);

    tick_curr_rect.ref.x.val = tick_curr_rect_x.get();

    const unsigned int music_length = static_cast<unsigned int>(music.GetTimeLength());
    const unsigned int music_time_int = static_cast<unsigned int>(music_time);
    const std::string music_time_str = std::format("{}:{:02} / {}:{:02}", music_time_int / 60, music_time_int % 60, music_length / 60, music_length % 60);
    music_time_text.ref.set_string(music_time_str);
}

void editor::set_tick_freq_idx(std::size_t new_tick_freq_idx)
{
    tick_freq_idx = new_tick_freq_idx;
    tick_text.ref.set_string("1 / " + std::to_string(tick_freqs[tick_freq_idx]));

    const float new_y = static_cast<float>(tick_freq_idx * 2 + 1) / (tick_freq_count * 2);
    tick_curr_rect_x.set(std::nullopt, new_y, 0.2f, kee::transition_type::exp);
}

} // namespace scene
} // namespace kee