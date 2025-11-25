#include "kee/scene/editor/compose_tab.hpp"

#include <chrono>
#include <ranges>

#include "kee/scene/editor/root.hpp"

namespace kee {
namespace scene {
namespace editor {

editor_hit_object::editor_hit_object(int key, float duration) :
    key(key),
    duration(duration)
{ }

hit_obj_ui::hit_obj_ui(const kee::ui::required& reqs, float beat, float duration, float curr_beat, float beat_width, std::size_t key_idx, std::size_t rendered_key_count) :
    kee::ui::rect(reqs,
        raylib::Color::DarkBlue(),
        pos(pos::type::rel, (beat - curr_beat + beat_width) / (2 * beat_width)),
        pos(pos::type::rel, object_editor::hit_objs_rel_y + object_editor::hit_objs_rel_h * (key_idx + 1) / (rendered_key_count + 1) - hit_obj_ui::rel_h / 2),
        dims(
            dim(dim::type::rel, duration / (2 * beat_width)),
            dim(dim::type::rel, hit_obj_ui::rel_h)
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
    )),
    start_key_idx(key_idx),
    selected(false),
    rendered_key_count(rendered_key_count),
    key_idx(key_idx)
{ }

void hit_obj_ui::select()
{
    selected = true;

    set_opt_color(raylib::Color::DarkGreen());
    border.value().opt_color = raylib::Color::Green();
    circle_l.ref.set_opt_color(raylib::Color::Green());
    circle_r.ref.set_opt_color(raylib::Color::Green());
}

void hit_obj_ui::unselect()
{
    selected = false;

    set_opt_color(raylib::Color::DarkBlue());
    border.value().opt_color = raylib::Color::Blue();
    circle_l.ref.set_opt_color(raylib::Color::Blue());
    circle_r.ref.set_opt_color(raylib::Color::Blue());
}

bool hit_obj_ui::is_selected() const
{
    return selected;
}

std::size_t hit_obj_ui::get_key_idx() const
{
    return key_idx;
}

void hit_obj_ui::set_key_idx(std::size_t new_key_idx)
{
    y.val = object_editor::hit_objs_rel_y + object_editor::hit_objs_rel_h * (new_key_idx + 1) / (rendered_key_count + 1) - hit_obj_ui::rel_h / 2;
    key_idx = new_key_idx;
}

void hit_obj_ui::update_pos_x(float beat, float duration, float curr_beat, float beat_width)
{
    x.val = (beat - curr_beat + beat_width) / (2 * beat_width);
    std::get<kee::dims>(dimensions).w.val = duration / (2 * beat_width);
}

hit_obj_position::hit_obj_position(float beat, float duration) :
    beat(beat),
    duration(duration)
{ }

hit_obj_metadata::hit_obj_metadata(int key, float beat, float duration) :
    key(key),
    position(beat, duration)
{ }

new_hit_obj_data::new_hit_obj_data(int key, std::size_t key_idx, float click_beat, float current_beat, bool from_compose_tab) :
    key(key),
    key_idx(key_idx),
    click_beat(click_beat),
    current_beat(current_beat),
    from_compose_tab(from_compose_tab)
{ }

compose_tab_event::compose_tab_event(
    const std::vector<int>& selected_key_ids, 
    const std::vector<hit_obj_metadata>& added, 
    const std::vector<hit_obj_metadata>& removed
) :
    selected_key_ids(selected_key_ids),
    added(added),
    removed(removed)
{ }

compose_tab_key::compose_tab_key(
    const kee::ui::required& reqs, 
    compose_tab& compose_tab_scene,
    song_ui& song_ui_elem,
    std::map<float, editor_hit_object>& hit_objects,
    int key_id
) :
    kee::ui::button(reqs,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, kee::key_border_parent_h),
        true
    ),
    hit_objects(hit_objects),
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
    compose_tab_scene(compose_tab_scene),
    song_ui_elem(song_ui_elem),
    key_id(key_id)
{
    on_event = [&](ui::button::event button_event, magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_down_l:
            if (!mods.test(kee::mods::ctrl))
                this->compose_tab_scene.unselect();

            this->compose_tab_scene.select(this->key_id);
            break;
        case ui::button::event::on_down_r: {
            this->compose_tab_scene.unselect();
            this->compose_tab_scene.select(this->key_id);

            this->compose_tab_scene.obj_editor.ref.new_hit_object = new_hit_obj_data(this->key_id, 0, this->song_ui_elem.get_beat(), this->song_ui_elem.get_beat(), false);
            break;
        }
        case ui::button::event::on_leave:
            if (this->compose_tab_scene.obj_editor.ref.new_hit_object.has_value())
            {
                this->compose_tab_scene.unselect();
                this->compose_tab_scene.obj_editor.ref.new_hit_object.reset();
            }
            break;
        default:
            break;
        }
    };

    on_click_r = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        this->compose_tab_scene.obj_editor.ref.attempt_add_hit_obj();
    };

    set_opt_color(raylib::Color::White());

    const std::string key_str = (key_id != KeyboardKey::KEY_SPACE)
        ? std::string(1, static_cast<char>(key_id))
        : "___";

    key_text.ref.set_string(key_str);
}

void compose_tab_key::update_element([[maybe_unused]] float dt)
{
    hit_obj_rects.clear();
    for (const auto& [beat, object] : hit_objects)
    {
        if (beat + object.duration < song_ui_elem.get_beat() - compose_tab::beat_lock_threshold)
            continue;

        if (beat > song_ui_elem.get_beat() + compose_tab_scene.approach_beats)
            break;

        const float start_progress = std::max((beat - song_ui_elem.get_beat()) / (2 * compose_tab_scene.approach_beats), 0.0f);
        const float end_progress = std::max((beat + object.duration - song_ui_elem.get_beat()) / (2 * compose_tab_scene.approach_beats), 0.0f);
        hit_obj_rects.push_back(make_temp_child<kee::ui::rect>(
            raylib::Color::Blank(),
            pos(pos::type::rel, 0.5),
            pos(pos::type::rel, 0.5),
            border(border::type::rel_h, start_progress),
            true,
            ui::rect_outline(ui::rect_outline::type::abs, 0, raylib::Color::Red()),
            std::nullopt
        ));

        const float rel_h_parent = std::max(end_progress - start_progress, kee::key_border_width);
        hit_obj_rects.back().border.value().val = get_raw_rect().height * rel_h_parent;
    }
}

void compose_tab_key::render_element() const
{
    for (const kee::ui::rect& hit_obj_rect : hit_obj_rects)
        hit_obj_rect.render();
}

selection_box_info::selection_box_info(kee::ui::rect&& rect, float rel_y_start) :
    rect(std::move(rect)),
    rel_y_start(rel_y_start)
{ }

selection_key_drag_info::selection_key_drag_info(std::size_t key_idx_lo, std::size_t key_idx_hi, std::size_t key_idx_start) :
    key_idx_lo(key_idx_lo),
    key_idx_hi(key_idx_hi),
    key_idx_start(key_idx_start)
{ }

selection_obj_info::selection_obj_info(const std::optional<selection_key_drag_info>& key_idx_info, drag_selection hit_obj_drag_selection) :
    key_idx_info(key_idx_info),
    hit_obj_drag_selection(hit_obj_drag_selection),
    selected_has_moved(false)
{ }

selection_info::selection_info(
    float beat_drag_start, 
    raylib::Vector2 mouse_pos_start,
    std::variant<selection_box_info, selection_obj_info>&& variant
) :
    beat_drag_start(beat_drag_start),
    mouse_pos_start(mouse_pos_start),
    variant(std::move(variant)),
    has_moved(false)
{ }

hit_obj_ui_key::hit_obj_ui_key(std::map<float, editor_hit_object>& map, std::map<float, editor_hit_object>::iterator it) :
    map(map),
    it(it)
{ }

hit_obj_metadata hit_obj_ui_key::get_metadata() const
{
    if (!map.has_value())
        throw std::runtime_error("get_metadata: is extracted and not valid");

    return hit_obj_metadata(it->second.key, it->first, it->second.duration);
}

void hit_obj_ui_key::delete_from_map()
{
    if (!map.has_value())
        throw std::runtime_error("delete_from_map: is extracted and not valid");

    map.value().erase(it);
    map.reset();
}

bool hit_obj_ui_cmp::operator()(const hit_obj_ui_key& l, const hit_obj_ui_key& r) const
{
    return (l.it->second.key != r.it->second.key)
        ? compose_tab::key_to_prio.at(l.it->second.key) < compose_tab::key_to_prio.at(r.it->second.key)
        : l.it->first < r.it->first;
}

bool hit_obj_ui_cmp::operator()(const hit_obj_ui_key& l, const hit_obj_metadata& r) const
{
    return (l.it->second.key != r.key)
        ? compose_tab::key_to_prio.at(l.it->second.key) < compose_tab::key_to_prio.at(r.key)
        : l.it->first < r.position.beat;
}

bool hit_obj_ui_cmp::operator()(const hit_obj_metadata& l, const hit_obj_ui_key& r) const
{
    return (r.it->second.key != l.key)
        ? compose_tab::key_to_prio.at(r.it->second.key) < compose_tab::key_to_prio.at(l.key)
        : r.it->first < l.position.beat;
}

object_editor::object_editor(
    const kee::ui::required& reqs,
    const std::vector<int>& selected_key_ids,
    std::unordered_map<int, kee::ui::handle<compose_tab_key>>& keys,
    compose_tab& compose_tab_scene,
    song_ui& song_ui_elem
) :
    kee::ui::rect(reqs,
        raylib::Color(15, 15, 15, 255),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 1.0f),
            dim(dim::type::rel, 0.25f)
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
    compose_tab_scene(compose_tab_scene),
    song_ui_elem(song_ui_elem),
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
    beat_drag_multiplier(0)
{ }

void object_editor::reset_render_hit_objs()
{
    const std::vector<int>& keys_to_render = get_keys_to_render();

    obj_render_info.clear();
    for (std::size_t i = keys_to_render.size(); i-- > 0;)
    for (auto it = keys.at(keys_to_render[i]).ref.hit_objects.begin(); it != keys.at(keys_to_render[i]).ref.hit_objects.end(); it++)
    {
        auto& [beat, object] = *it;
        
        hit_obj_ui render_ui = make_temp_child<hit_obj_ui>(beat, object.duration, song_ui_elem.get_beat(), object_editor::beat_width, i, keys_to_render.size());
        obj_render_info.emplace(hit_obj_ui_key(keys.at(keys_to_render[i]).ref.hit_objects, it), std::move(render_ui));
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
    if (selection.has_value())
        return false;

    std::vector<hit_obj_metadata> deleted_objs;
    for (const auto& [key, ui] : obj_render_info)
        if (ui.is_selected())
            deleted_objs.push_back(key.get_metadata());

    compose_tab_scene.add_event(compose_tab_event(selected_key_ids, std::vector<hit_obj_metadata>(), deleted_objs));
    return true;
}

void object_editor::attempt_add_hit_obj()
{
    const float new_beat = std::min(new_hit_object.value().click_beat, new_hit_object.value().current_beat);
    const float new_duration = std::max(new_hit_object.value().click_beat, new_hit_object.value().current_beat) - new_beat;

    if (!new_hit_object.value().from_compose_tab)
        compose_tab_scene.unselect();
    
    std::vector<hit_obj_metadata> new_obj_added;
    new_obj_added.emplace_back(new_hit_object.value().key, new_beat, new_duration);
    compose_tab_scene.add_event(compose_tab_event(selected_key_ids, new_obj_added, std::vector<hit_obj_metadata>()));

    new_hit_object.reset();
}

const std::vector<int>& object_editor::get_keys_to_render() const
{
    return selected_key_ids.empty() ? compose_tab::prio_to_key : selected_key_ids;
}

void object_editor::on_element_mouse_move(const raylib::Vector2& mouse_pos, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    if (compose_tab_scene.is_music_playing())
        return;

    const raylib::Rectangle obj_editor_raw_rect = get_raw_rect();
    const float mouse_in_rect_percent = (mouse_pos.x - obj_editor_raw_rect.x) / obj_editor_raw_rect.width;
    const float mouse_beat_diff = (mouse_in_rect_percent - 0.5f) * beat_width * 2;
    mouse_beat = song_ui_elem.get_beat() + mouse_beat_diff;

    if (beat_hover_l.ref.get_raw_rect().CheckCollision(mouse_pos))
        beat_drag_multiplier = -1;
    else if (beat_hover_r.ref.get_raw_rect().CheckCollision(mouse_pos))
        beat_drag_multiplier = 1;
    else
        beat_drag_multiplier = 0;

    if (selection.has_value() && std::holds_alternative<selection_box_info>(selection.value().variant))
    {
        const float selection_start_beat = std::min(mouse_beat, selection.value().beat_drag_start);
        const float selection_end_beat = std::max(mouse_beat, selection.value().beat_drag_start);
        const float selection_mouse_rel_y = (mouse_pos.y - obj_editor_raw_rect.y) / obj_editor_raw_rect.height;

        selection_box_info& box_selection = std::get<selection_box_info>(selection.value().variant);
        float selection_start_rel_y = std::min(selection_mouse_rel_y, box_selection.rel_y_start);
        float selection_end_rel_y = std::max(selection_mouse_rel_y, box_selection.rel_y_start);

        if (selection_start_rel_y < 0)
            selection_start_rel_y = 0;
        if (selection_end_rel_y > 1)
            selection_end_rel_y = 1;

        box_selection.rect.x.val = (selection_start_beat - (song_ui_elem.get_beat() - beat_width)) / (2 * beat_width);
        box_selection.rect.y.val = selection_start_rel_y;

        auto& [w, h] = std::get<kee::dims>(box_selection.rect.dimensions);
        w.val = (selection_end_beat - selection_start_beat) / (2 * beat_width);
        h.val = selection_end_rel_y - selection_start_rel_y;
    }
    else if (new_hit_object.has_value())
    { 
        float current_beat;
        if (new_hit_object.value().from_compose_tab)
        {
            current_beat = mouse_beat;
            if (compose_tab_scene.is_beat_snap_enabled())
                current_beat = std::round(current_beat * compose_tab_scene.get_ticks_per_beat()) / compose_tab_scene.get_ticks_per_beat();
        }
        else
            current_beat = song_ui_elem.get_beat();

        new_hit_object.value().current_beat = current_beat;
    }

    if (selection.has_value())
    {
        if (!selection.value().has_moved && mouse_pos != selection.value().mouse_pos_start)
        {
            if (std::holds_alternative<selection_obj_info>(selection.value().variant) && std::abs(std::fmod(selected_reference_beat, 1.0f / compose_tab_scene.get_ticks_per_beat())) > compose_tab_scene.beat_lock_threshold && compose_tab_scene.is_beat_snap_enabled())
                selected_reference_beat = std::round(selected_reference_beat * compose_tab_scene.get_ticks_per_beat()) / compose_tab_scene.get_ticks_per_beat();

            std::println("{}", selected_reference_beat);
            selection.value().has_moved = true;
        }

        if (std::holds_alternative<selection_obj_info>(selection.value().variant) && std::get<selection_obj_info>(selection.value().variant).key_idx_info.has_value())
        {
            const std::vector<int>& keys_to_render = get_keys_to_render();
            const selection_obj_info& obj_selection = std::get<selection_obj_info>(selection.value().variant);
            const std::ptrdiff_t key_idx_delta = std::clamp(
                static_cast<std::ptrdiff_t>(get_mouse_key_idx(mouse_pos.y)) - static_cast<std::ptrdiff_t>(obj_selection.key_idx_info.value().key_idx_start), 
                static_cast<std::ptrdiff_t>(obj_selection.key_idx_info.value().key_idx_lo) * -1,
                static_cast<std::ptrdiff_t>(keys_to_render.size() - 1) - static_cast<std::ptrdiff_t>(obj_selection.key_idx_info.value().key_idx_hi)
            );
            
            for (auto& [key, val] : obj_render_info)
                if (val.is_selected())
                    val.set_key_idx(val.start_key_idx + key_idx_delta);
        }
    }
}

bool object_editor::on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods)
{
    const raylib::Rectangle obj_renderer_rect = obj_renderer.ref.get_raw_rect();
    if (!obj_renderer_rect.CheckCollision(mouse_pos) || compose_tab_scene.is_music_playing())
        return false;

    if (!is_mouse_l)
    {
        const std::size_t selected_key_idx = get_mouse_key_idx(mouse_pos.y);
        float start_beat = mouse_beat;
        if (compose_tab_scene.is_beat_snap_enabled())
            start_beat = std::round(start_beat * compose_tab_scene.get_ticks_per_beat()) / compose_tab_scene.get_ticks_per_beat();

        const std::vector<int>& keys_to_render = selected_key_ids.empty() ? compose_tab::prio_to_key : selected_key_ids;
        new_hit_object = new_hit_obj_data(keys_to_render[selected_key_idx], selected_key_idx, start_beat, start_beat, true);
        return true;
    }

    auto obj_rects_clicked = obj_render_info | std::views::filter(
        [&mouse_pos](const auto& element)
        { 
            return element.second.get_extended_raw_rect().CheckCollision(mouse_pos); 
        }
    );

    /**
     * If two hit objects intersect with mouse position on-click, already selected objects take
     * priority. Then the top-rightmost one (top-most prioritized over right-most) gets selected 
     * since that is how hit objects are ordered when rendered.
     */
    auto obj_selected_it = std::ranges::max_element(obj_rects_clicked,
        [](const auto& l, const auto& r) -> bool
        {
            if (l.second.is_selected() != r.second.is_selected())
                return r.second.is_selected();

            static const hit_obj_ui_cmp cmp;
            return cmp(l.first, r.first);
        }
    );
    
    if (obj_selected_it != std::ranges::end(obj_rects_clicked))
    {
        bool is_drag_possible = true;
        if (!obj_selected_it->second.is_selected())
        {
            if (mods.test(kee::mods::ctrl))
                is_drag_possible = false;
            else
                for (auto& [_, val] : obj_render_info)
                    if (val.is_selected())
                        val.unselect();

            obj_selected_it->second.select();
        }
        else
        {
            is_drag_possible = std::none_of(obj_render_info.begin(), obj_render_info.end(), 
                [&](const auto& obj) 
                {
                    const bool distinct_from_selected = &obj != &(*obj_selected_it);
                    return distinct_from_selected && obj.second.is_selected();
                }
            );
        }

        drag_selection hit_obj_drag_selection = drag_selection::none;
        std::optional<selection_key_drag_info> key_idx_info;

        if (is_drag_possible && obj_selected_it->first.get_metadata().position.duration > compose_tab::beat_lock_threshold)
        {
            const bool collide_with_l_dot = obj_selected_it->second.circle_l.ref.get_raw_rect().CheckCollision(mouse_pos);
            const bool collide_with_r_dot = obj_selected_it->second.circle_r.ref.get_raw_rect().CheckCollision(mouse_pos);

            if (collide_with_l_dot || collide_with_r_dot)
            {
                const float selected_obj_midpoint_x = obj_selected_it->second.get_raw_rect().x + obj_selected_it->second.get_raw_rect().width / 2;
                const bool is_left = (collide_with_l_dot && collide_with_r_dot)
                    ? mouse_pos.x < selected_obj_midpoint_x
                    : collide_with_l_dot;

                hit_obj_drag_selection = is_left 
                    ? drag_selection::drag_l
                    : drag_selection::drag_r;
            }
        }

        if (hit_obj_drag_selection == drag_selection::none && !compose_tab_scene.is_key_lock_enabled())
        {
            std::size_t key_idx_lo = std::numeric_limits<std::size_t>::max();
            std::size_t key_idx_hi = std::numeric_limits<std::size_t>::min();

            for (auto& [key, val] : obj_render_info)
                if (val.is_selected())
                {
                    if (key_idx_lo > val.start_key_idx)
                        key_idx_lo = val.start_key_idx;

                    if (key_idx_hi < val.start_key_idx)
                        key_idx_hi = val.start_key_idx;
                }

            key_idx_info.emplace(key_idx_lo, key_idx_hi, get_mouse_key_idx(mouse_pos.y));
        }

        selected_beat = std::numeric_limits<float>::max();
        for (auto& [key, val] : obj_render_info)
            if (selected_beat > key.get_metadata().position.beat && val.is_selected())
                selected_beat = key.get_metadata().position.beat;

        selected_reference_beat = (hit_obj_drag_selection == drag_selection::drag_r) 
            ? obj_selected_it->first.get_metadata().position.beat + obj_selected_it->first.get_metadata().position.duration
            : selected_beat;

        const selection_obj_info obj_selection = selection_obj_info(key_idx_info, hit_obj_drag_selection);
        selection.emplace(mouse_beat, mouse_pos, obj_selection);
    }
    else
    {
        for (auto& [key, val] : obj_render_info)
            if (val.is_selected())
                val.unselect();

        const float rel_start_y = (mouse_pos.y - obj_renderer_rect.y) / obj_renderer_rect.height;
        selection_box_info box_selection = selection_box_info(
            make_temp_child<kee::ui::rect>(
                raylib::Color(255, 255, 255, 50),
                pos(pos::type::rel, 0),
                pos(pos::type::rel, 0),
                dims(
                    dim(dim::type::rel, 0),
                    dim(dim::type::rel, 0)
                ),
                false, std::nullopt, std::nullopt
            ),
            rel_start_y
        );

        selection.emplace(mouse_beat, mouse_pos, std::move(box_selection));
    }
    
    return true;
}

bool object_editor::on_element_mouse_up([[maybe_unused]] const raylib::Vector2& mouse_pos, bool is_mouse_l, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    if (compose_tab_scene.is_music_playing() || (!selection.has_value() && !new_hit_object.has_value()) || is_mouse_l == new_hit_object.has_value())
        return false;

    handle_mouse_up(is_mouse_l);
    selection.reset();
    return true;
}

void object_editor::update_element([[maybe_unused]] float dt)
{
    const float beat_drag_diff = get_beat_drag_diff();
    for (auto& map_elem : obj_render_info)
    {
        hit_obj_position update = hit_obj_update_info(map_elem, beat_drag_diff);
        map_elem.second.update_pos_x(update.beat, update.duration, song_ui_elem.get_beat(), beat_width);
    }

    const int beat_start_tick = static_cast<int>(std::floor(song_ui_elem.get_beat() - beat_width) * compose_tab_scene.get_ticks_per_beat());
    const int beat_end_tick = static_cast<int>(std::ceil(song_ui_elem.get_beat() + beat_width) * compose_tab_scene.get_ticks_per_beat());
    
    beat_render_rects.clear();
    whole_beat_texts.clear();
    for (int beat_tick = beat_start_tick; beat_tick <= beat_end_tick; beat_tick++)
    {
        const float beat_render = static_cast<float>(beat_tick) / compose_tab_scene.get_ticks_per_beat();
        const float render_rel_x = (beat_render - (song_ui_elem.get_beat() - beat_width)) / (2 * beat_width);

        const bool is_whole_beat = beat_tick % compose_tab_scene.get_ticks_per_beat() == 0;
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
            const int whole_beat = beat_tick / compose_tab_scene.get_ticks_per_beat();
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
        
        new_hit_obj_render.emplace(make_temp_child<hit_obj_ui>(beg_beat, end_beat - beg_beat, song_ui_elem.get_beat(), beat_width, new_hit_object.value().key_idx, get_keys_to_render().size()));
        new_hit_obj_render.value().select();
    }

    const float beat_delta = beat_drag_multiplier * beat_drag_speed * dt;
    song_ui_elem.set_beat(song_ui_elem.get_beat() + beat_delta);
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

    for (const auto& [_, val] : obj_render_info)
        if (val.get_extended_raw_rect().CheckCollision(obj_renderer.ref.get_raw_rect()) && !val.is_selected())
            val.render();

    for (const auto& [_, val] : obj_render_info)
        if (val.get_extended_raw_rect().CheckCollision(obj_renderer.ref.get_raw_rect()) && val.is_selected())
            val.render();

    if (selection.has_value() && std::holds_alternative<selection_box_info>(selection.value().variant))
        std::get<selection_box_info>(selection.value().variant).rect.render();
    else if (new_hit_object.has_value())
        new_hit_obj_render.value().render();
}

float object_editor::get_beat_drag_diff() const
{
    if (selection.has_value() && std::holds_alternative<selection_obj_info>(selection.value().variant))
    {
        float beat_drag_diff = mouse_beat - selection.value().beat_drag_start;
        if (compose_tab_scene.is_beat_snap_enabled())
            beat_drag_diff = std::round(beat_drag_diff * compose_tab_scene.get_ticks_per_beat()) / compose_tab_scene.get_ticks_per_beat();
    
        return beat_drag_diff;
    }
    else
        return 0.0f;
}

hit_obj_position object_editor::hit_obj_update_info(const std::pair<const hit_obj_ui_key, hit_obj_ui>& map_elem, float beat_drag_diff) const
{
    const hit_obj_position pos = map_elem.first.get_metadata().position;
    float update_beat = pos.beat;
    float update_duration = pos.duration;

    if (map_elem.second.is_selected() && selection.has_value() && std::holds_alternative<selection_obj_info>(selection.value().variant))
    {
        const selection_obj_info& obj_selection = std::get<selection_obj_info>(selection.value().variant);

        switch (obj_selection.hit_obj_drag_selection)
        {
        case drag_selection::none:
            update_beat = selected_reference_beat + beat_drag_diff + pos.beat - selected_beat;
            break;
        case drag_selection::drag_l: {
            float max_hit_obj_beat = std::floor((pos.beat + pos.duration) * compose_tab_scene.get_ticks_per_beat()) / compose_tab_scene.get_ticks_per_beat();
            if (std::abs(pos.beat + pos.duration - max_hit_obj_beat) <= compose_tab::beat_lock_threshold)
                max_hit_obj_beat -= 1.0f / compose_tab_scene.get_ticks_per_beat();

            update_beat = std::min(selected_reference_beat + beat_drag_diff, max_hit_obj_beat);
            update_duration = (pos.beat + pos.duration) - update_beat;
            break;
        }
        case drag_selection::drag_r: {
            float min_hit_obj_beat = std::ceil(pos.beat * compose_tab_scene.get_ticks_per_beat()) / compose_tab_scene.get_ticks_per_beat();
            if (std::abs(pos.beat - min_hit_obj_beat) <= compose_tab::beat_lock_threshold)
                min_hit_obj_beat += 1.0f / compose_tab_scene.get_ticks_per_beat();

            const float update_end_beat = std::max(selected_reference_beat + beat_drag_diff, min_hit_obj_beat);
            update_duration = update_end_beat - pos.beat;
            break;
        }}
    }

    return hit_obj_position(update_beat, update_duration);
}

void object_editor::handle_mouse_up(bool is_mouse_l)
{
    if (!is_mouse_l)
    {
        attempt_add_hit_obj();
        return;
    }
    
    if (selection.has_value() && std::holds_alternative<selection_box_info>(selection.value().variant))
    {
        if (!selection.value().has_moved)
            return;

        const raylib::Rectangle selection_raw_rect = std::get<selection_box_info>(selection.value().variant).rect.get_raw_rect();
        selected_beat = std::numeric_limits<float>::max();

        for (auto& [key, val] : obj_render_info)
            if (selection_raw_rect.CheckCollision(val.get_extended_raw_rect()))
            {
                val.select();
                selected_beat = std::min(selected_beat, key.get_metadata().position.beat);
            }

        selected_reference_beat = selected_beat;
        return;
    }

    attempt_move_op();
    selected_beat = selected_reference_beat;
}

void object_editor::attempt_move_op()
{
    const std::vector<int>& keys_to_render = get_keys_to_render();

    std::vector<hit_obj_metadata> hit_objs_added;
    std::vector<hit_obj_metadata> hit_objs_removed;

    bool same_check = false;
    for (auto it = obj_render_info.begin(); it != obj_render_info.end(); it++)
        if (it->second.is_selected())
        {
            const hit_obj_position new_obj_position = hit_obj_update_info(*it, get_beat_drag_diff());
            const hit_obj_metadata new_obj = hit_obj_metadata(keys_to_render[it->second.get_key_idx()], new_obj_position.beat, new_obj_position.duration);
            const hit_obj_metadata old_obj = it->first.get_metadata();

            hit_objs_added.push_back(new_obj);
            hit_objs_removed.push_back(old_obj);
            if (!same_check)
            {
                if (new_obj.key == old_obj.key && new_obj.position == old_obj.position)
                    return;

                same_check = true;
            }
        }

    compose_tab_scene.add_event(compose_tab_event(selected_key_ids, hit_objs_added, hit_objs_removed));
}

std::size_t object_editor::get_mouse_key_idx(float mouse_pos_y) const
{
    const raylib::Rectangle obj_renderer_rect = obj_renderer.ref.get_raw_rect();
    const float mouse_rel_y = (mouse_pos_y - obj_renderer_rect.y) / obj_renderer_rect.height;

    const std::vector<int>& keys_to_render = get_keys_to_render();
    const float key_rel_y_range = object_editor::hit_objs_rel_h / (keys_to_render.size() + 1);
    const float keys_rel_y_beg = object_editor::hit_objs_rel_y + object_editor::hit_objs_rel_h * 1 / (keys_to_render.size() + 1) - key_rel_y_range / 2;
    const float keys_rel_y_end = object_editor::hit_objs_rel_y + object_editor::hit_objs_rel_h * keys_to_render.size() / (keys_to_render.size() + 1) + key_rel_y_range / 2;

    if (mouse_rel_y < keys_rel_y_beg)
        return 0;
    else if (mouse_rel_y > keys_rel_y_end)
        return keys_to_render.size() - 1;
    else
        return static_cast<std::size_t>((mouse_rel_y - keys_rel_y_beg) / key_rel_y_range);
}

const std::vector<int> compose_tab::prio_to_key = []
{
    std::vector<int> res;
    for (const kee::key_pos_data& data : kee::key_ui_data)
        res.push_back(data.raylib_key);
    return res;
}();

const std::unordered_map<int, int> compose_tab::key_to_prio = []
{
    std::unordered_map<int, int> res;
    for (std::size_t i = 0; i < kee::key_ui_data.size(); i++)
        res.emplace(kee::key_ui_data[i].raylib_key, static_cast<int>(i));
    return res;
}();

compose_tab_info::compose_tab_info(
    const kee::image_texture& arrow_png,
    const std::optional<boost::json::object>& keys_json_obj
) :
    arrow_png(arrow_png),
    hitsound("assets/sfx/hitsound.wav"),
    is_beat_snap(true),
    is_key_locked(true),
    events_since_save(0), 
    event_history_idx(0),
    tick_freq_idx(3)
{
    for (const auto& [id, _] : kee::key_ui_data)
    {
        hit_objs[id];

        if (keys_json_obj.has_value())
        {
            const std::string key_str = std::string(1, static_cast<char>(id));
            const boost::json::array& key_hit_objs = keys_json_obj.value().at(key_str).as_array();
            
            for (const boost::json::value& key_hit_obj : key_hit_objs)
            {
                const boost::json::object& key_hit_obj_json = key_hit_obj.as_object();
                const float beat = static_cast<float>(key_hit_obj_json.at("beat").as_double());
                const float duration = static_cast<float>(key_hit_obj_json.at("duration").as_double());
                
                hit_objs.at(id).emplace(beat, editor_hit_object(id, duration));
            }
        }
    }

    hitsound.SetVolume(0.01f);
}

compose_tab::compose_tab(const kee::ui::required& reqs, const float& approach_beats, song_ui& song_ui_elem, compose_tab_info& compose_info) :
    kee::ui::base(reqs,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.04f),
        dims(
            dim(dim::type::rel, 1.0f),
            dim(dim::type::rel, 0.88f)
        ),
        false
    ),
    approach_beats(approach_beats),
    obj_editor(add_child<object_editor>(std::nullopt, selected_key_ids, keys, *this, song_ui_elem)),
    song_ui_elem(song_ui_elem),
    compose_info(compose_info),
    beat_snap_button_color(add_transition<kee::color>(kee::color::white)),
    beat_snap_button_outline(add_transition<float>(compose_info.is_beat_snap ? 0.6f : 0.2f)),
    key_lock_button_color(add_transition<kee::color>(kee::color::white)),
    key_lock_button_outline(add_transition<float>(compose_info.is_key_locked ? 0.6f : 0.2f)),
    tick_l_button_color(add_transition<kee::color>(kee::color::white)),
    tick_r_button_color(add_transition<kee::color>(kee::color::white)),
    tick_l_button_scale(add_transition<float>(1.0f)),
    tick_r_button_scale(add_transition<float>(1.0f)),
    tick_curr_rect_x(add_transition<float>(static_cast<float>(compose_info.tick_freq_idx * 2 + 1) / (compose_tab_info::tick_freq_count * 2))),
    inspector_rect(add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(20, 20, 20),
        pos(pos::type::rel, 0.8f),
        pos(pos::type::rel, 0.25f),
        dims(
            dim(dim::type::rel, 0.2f),
            dim(dim::type::rel, 0.75f)
        ),
        false, std::nullopt, std::nullopt
    )),
    beat_snap_button(inspector_rect.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.25f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.025f)
        ),
        false
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
        kee::ui::rect_outline(kee::ui::rect_outline::type::rel_h, beat_snap_button_outline.get(), raylib::Color::White()),
        std::nullopt
    )),
    beat_snap_text(inspector_rect.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.23f),
        pos(pos::type::rel, 0.245f),
        ui::text_size(ui::text_size::type::rel_h, 0.035f),
        false, assets.font_semi_bold, "BEAT SNAP", false
    )),
    key_lock_button(inspector_rect.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.32f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.025f)
        ),
        false
    )),
    key_lock_button_rect(key_lock_button.ref.add_child<kee::ui::rect>(std::nullopt,
        raylib::Color::Blank(),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        false,
        kee::ui::rect_outline(kee::ui::rect_outline::type::rel_h, key_lock_button_outline.get(), raylib::Color::White()),
        std::nullopt
    )),
    key_lock_text(inspector_rect.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.23f),
        pos(pos::type::rel, 0.315f),
        ui::text_size(ui::text_size::type::rel_h, 0.035f),
        false, assets.font_semi_bold, "KEY LOCK", false
    )),
    tick_text(inspector_rect.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.08f),
        ui::text_size(ui::text_size::type::rel_h, 0.07f),
        true, assets.font_semi_bold, "1 / " + std::to_string(get_ticks_per_beat()), false
    )),
    tick_l_button(inspector_rect.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.2f),
        pos(pos::type::rel, 0.08f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.04f)
        ),
        true
    )),
    tick_l_img(tick_l_button.ref.add_child<kee::ui::image>(std::nullopt,
        compose_info.arrow_png, raylib::Color::White(),
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
        pos(pos::type::rel, 0.08f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.04f)
        ),
        true
    )),
    tick_r_img(tick_r_button.ref.add_child<kee::ui::image>(std::nullopt,
        compose_info.arrow_png, raylib::Color::White(),
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
        pos(pos::type::rel, 0.15f),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.04f)
        ),
        false, std::nullopt, std::nullopt
    )),
    tick_curr_rect(tick_frame.ref.add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(40, 40, 40, 255),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::rel, 1.0f / compose_tab_info::tick_freq_count),
            dim(dim::type::rel, 1)
        ),
        true, std::nullopt, std::nullopt
    )),
    key_border(add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.4f),
        pos(pos::type::rel, 0.625f),
        dims(
            dim(dim::type::rel, 0.75f),
            dim(dim::type::rel, 0.6f)
        ),
        true
    )),
    key_frame(key_border.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 11),
            dim(dim::type::aspect, 4)
        ),
        true
    )),
    mouse_wheel_move(0.0f)
{
    for (std::size_t i = 0; i < compose_tab_info::tick_freq_count; i++)
    {
        tick_frame_texts.push_back(tick_frame.ref.add_child<kee::ui::text>(std::nullopt,
            raylib::Color::White(),
            pos(pos::type::rel, static_cast<float>(i * 2 + 1) / (compose_tab_info::tick_freq_count * 2)),
            pos(pos::type::rel, 0.5f),
            ui::text_size(ui::text_size::type::rel_h, 0.7f),
            true, assets.font_regular, std::to_string(compose_tab_info::tick_freqs[i]), false
        ));

        tick_frame_buttons.push_back(tick_frame.ref.add_child<kee::ui::button>(std::nullopt,
            pos(pos::type::rel, static_cast<float>(i * 2 + 1) / (compose_tab_info::tick_freq_count * 2)),
            pos(pos::type::rel, 0.5f),
            dims(
                dim(dim::type::rel, 1.0f / compose_tab_info::tick_freq_count),
                dim(dim::type::rel, 1)
            ),
            true
        ));

        tick_frame_buttons.back().ref.on_event = [&, idx = i](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
        {
            switch (button_event)
            {
            case ui::button::event::on_hot:
                this->tick_frame_text_colors[idx].get().set(std::nullopt, kee::color::dark_orange, 0.5f, kee::transition_type::exp);
                break;
            case ui::button::event::on_leave:
                this->tick_frame_text_colors[idx].get().set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
                break;
            default:
                break;
            }
        };

        tick_frame_buttons.back().ref.on_click_l = [&, idx = i]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
        {
            this->set_tick_freq_idx(idx);
        };

        tick_frame_text_colors.push_back(add_transition<kee::color>(kee::color::white));
    }

    tick_l_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->tick_l_button_color.set(std::nullopt, kee::color::dark_orange, 0.555f, kee::transition_type::exp);
            this->tick_l_button_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            this->tick_l_button_scale.set(std::nullopt, 0.9f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->tick_l_button_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
            this->tick_l_button_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    tick_l_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (this->compose_info.tick_freq_idx != 0)
            this->set_tick_freq_idx(this->compose_info.tick_freq_idx - 1);
    };

    tick_r_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->tick_r_button_color.set(std::nullopt, kee::color::dark_orange, 0.5f, kee::transition_type::exp);
            this->tick_r_button_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            this->tick_r_button_scale.set(std::nullopt, 0.9f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->tick_r_button_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
            this->tick_r_button_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    tick_r_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (this->compose_info.tick_freq_idx != compose_tab_info::tick_freq_count - 1)
            this->set_tick_freq_idx(this->compose_info.tick_freq_idx + 1);
    };

    beat_snap_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (this->compose_info.is_beat_snap)
        {
            this->beat_snap_button_outline.set(std::nullopt, 0.2f, 0.25f, kee::transition_type::exp);
            this->compose_info.is_beat_snap = false;
        }
        else
        {
            this->beat_snap_button_outline.set(std::nullopt, 0.6f, 0.25f, kee::transition_type::exp);
            this->compose_info.is_beat_snap = true;
        }
    };

    beat_snap_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->beat_snap_button_color.set(std::nullopt, kee::color::dark_orange, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->beat_snap_button_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    key_lock_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (this->compose_info.is_key_locked)
        {
            this->key_lock_button_outline.set(std::nullopt, 0.2f, 0.25f, kee::transition_type::exp);
            this->compose_info.is_key_locked = false;
        }
        else
        {
            this->key_lock_button_outline.set(std::nullopt, 0.6f, 0.25f, kee::transition_type::exp);
            this->compose_info.is_key_locked = true;
        }
    };

    key_lock_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->key_lock_button_color.set(std::nullopt, kee::color::dark_orange, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->key_lock_button_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    for (const auto& [id, rel_pos] : kee::key_ui_data)
    {
        key_holders.push_back(key_frame.ref.add_child<kee::ui::base>(std::nullopt,
            pos(pos::type::rel, rel_pos.x),
            pos(pos::type::rel, rel_pos.y),
            dims(
                dim(dim::type::aspect, id == KeyboardKey::KEY_SPACE ? 8.0f : 1.0f),
                dim(dim::type::rel, 0.25)
            ),
            true
        ));

        keys.emplace(id, key_holders.back().ref.add_child<compose_tab_key>(std::nullopt, *this, song_ui_elem, compose_info.hit_objs[id], id));
    }

    unselect();
}

int compose_tab::get_ticks_per_beat() const
{
    return compose_tab_info::tick_freqs[compose_info.tick_freq_idx];
}

bool compose_tab::is_music_playing() const
{
    return song_ui_elem.get_music().IsPlaying();
}

bool compose_tab::is_beat_snap_enabled() const
{
    return compose_info.is_beat_snap;
}

bool compose_tab::is_key_lock_enabled() const
{
    return compose_info.is_key_locked;
}

void compose_tab::unselect()
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

void compose_tab::select(int id)
{
    if (keys.at(id).ref.is_selected)
        return;

    keys.at(id).ref.set_opt_color(raylib::Color::Green());
    keys.at(id).ref.is_selected = true;
    keys.at(id).ref.frame.change_z_order(0);
    keys.at(id).ref.key_text.change_z_order(0);

    selected_key_ids.insert(
        std::lower_bound(selected_key_ids.begin(), selected_key_ids.end(), id,
            [](int l, int r) { return compose_tab::key_to_prio.at(l) <= compose_tab::key_to_prio.at(r); }
        ),
    id);

    obj_editor.ref.reset_render_hit_objs();
}

void compose_tab::add_event(const compose_tab_event& e)
{
    if (!process_event(e))
        return;

    compose_info.event_history.erase(compose_info.event_history.begin(), compose_info.event_history.begin() + compose_info.event_history_idx);
    compose_info.event_history.push_front(e);
    compose_info.event_history_idx = 0;

    if (compose_info.events_since_save.has_value() && compose_info.events_since_save.value() >= 0)
        compose_info.events_since_save.value()++;
    else
        compose_info.events_since_save.reset();
}

bool compose_tab::process_event(const compose_tab_event& e)
{
    if (selected_key_ids != e.selected_key_ids)
    {
        selected_key_ids = e.selected_key_ids; 
        obj_editor.ref.reset_render_hit_objs();
    }

    for (auto& [_, ui] : obj_editor.ref.obj_render_info)
        if (ui.is_selected())
            ui.unselect();

    for (const hit_obj_metadata& hit_obj : e.removed)
    {
        auto it = obj_editor.ref.obj_render_info.find(hit_obj);
        auto node = obj_editor.ref.obj_render_info.extract(it);
        node.key().delete_from_map();
    }

    bool is_valid = true;
    for (const hit_obj_metadata& hit_obj : e.added)
    {
        std::map<float, editor_hit_object>& hit_objects = compose_info.hit_objs.at(hit_obj.key);
        auto next_it = hit_objects.lower_bound(hit_obj.position.beat);
        
        if (next_it != hit_objects.end() && hit_obj.position.beat + hit_obj.position.duration + compose_tab::beat_lock_threshold >= next_it->first)
            is_valid = false;
        else if (next_it != hit_objects.begin() && std::prev(next_it)->first + std::prev(next_it)->second.duration + compose_tab::beat_lock_threshold >= hit_obj.position.beat)
            is_valid = false;

        if (!is_valid)
            break;
    }

    const std::vector<hit_obj_metadata>& to_add = is_valid ? e.added : e.removed;
    for (const hit_obj_metadata& hit_obj : to_add)
    {
        const std::vector<int>& keys_to_render = obj_editor.ref.get_keys_to_render();
        const auto new_key_it = std::ranges::find(keys_to_render, hit_obj.key);
        const std::size_t new_key_idx = std::ranges::distance(keys_to_render.begin(), new_key_it);

        hit_obj_ui render_ui = obj_editor.ref.make_temp_child<hit_obj_ui>(hit_obj.position.beat, hit_obj.position.duration, song_ui_elem.get_beat(), object_editor::beat_width, new_key_idx, keys_to_render.size());
        if (is_valid)
            render_ui.select();

        std::map<float, editor_hit_object>& hit_objects = keys.at(hit_obj.key).ref.hit_objects;
        auto new_it = hit_objects.emplace(hit_obj.position.beat, editor_hit_object(hit_obj.key, hit_obj.position.duration)).first;
        obj_editor.ref.obj_render_info.emplace(hit_obj_ui_key(hit_objects, new_it), std::move(render_ui));
    }

    return is_valid;
}

bool compose_tab::on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods)
{
    switch (keycode)
    {
    case KeyboardKey::KEY_SPACE:
        if (!mods.test(kee::mods::ctrl))
            return false;

        for (int prev_id : selected_key_ids)
            keys.at(prev_id).ref.set_opt_color(raylib::Color::White());

        unselect();
        return true;
    case KeyboardKey::KEY_A:
        if (!mods.test(kee::mods::ctrl))
            return false;

        for (auto& [key, val] : obj_editor.ref.obj_render_info)
            val.select();

        return true;
    case KeyboardKey::KEY_C:
        if (!mods.test(kee::mods::ctrl))
            return false;

        clipboard.clear();
        clipboard_reference_beat = std::numeric_limits<float>::max();

        for (auto& [key, val] : obj_editor.ref.obj_render_info)
            if (val.is_selected())
            {
                clipboard.push_back(key.get_metadata());
                if (clipboard_reference_beat > key.get_metadata().position.beat)
                    clipboard_reference_beat = key.get_metadata().position.beat;
            }

        return true;
    case KeyboardKey::KEY_V: {
        if (!mods.test(kee::mods::ctrl))
            return false;

        std::vector<hit_obj_metadata> pasted_objs;
        const float paste_beat = song_ui_elem.get_beat();
        for (const hit_obj_metadata& metadata : clipboard)
        {
            const float new_beat = paste_beat + metadata.position.beat - clipboard_reference_beat;
            pasted_objs.emplace_back(metadata.key, new_beat, metadata.position.duration);
        }

        add_event(compose_tab_event(selected_key_ids, pasted_objs, std::vector<hit_obj_metadata>()));
        return true;
    }
    case KeyboardKey::KEY_Z:
        if (!mods.test(kee::mods::ctrl))
            return false;

        if (mods.test(kee::mods::shift))
        {
            if (compose_info.event_history_idx <= 0)
                return true;

            std::swap(compose_info.event_history[compose_info.event_history_idx - 1].added, compose_info.event_history[compose_info.event_history_idx - 1].removed);
            process_event(compose_info.event_history[compose_info.event_history_idx - 1]);
            compose_info.event_history_idx--;

            if (compose_info.events_since_save.has_value())
                compose_info.events_since_save.value()++;
        }
        else
        {
            if (compose_info.event_history_idx >= compose_info.event_history.size())
                return true;

            std::swap(compose_info.event_history[compose_info.event_history_idx].added, compose_info.event_history[compose_info.event_history_idx].removed);
            process_event(compose_info.event_history[compose_info.event_history_idx]);
            compose_info.event_history_idx++;

            if (compose_info.events_since_save.has_value())
                compose_info.events_since_save.value()--;
        }
        
        return true;
    case KeyboardKey::KEY_BACKSPACE:
        return obj_editor.ref.delete_selected_hit_objs();
    default:
        return false;
    }
}

bool compose_tab::on_element_mouse_scroll(float mouse_scroll)
{
    if (is_music_playing())
        return false;

    mouse_wheel_move += mouse_scroll;
    if (std::abs(mouse_wheel_move) < 1.0f)
        return false;

    float new_beat = song_ui_elem.get_beat();
    float beat_steps = 0;

    if (mouse_wheel_move <= -1.0f)
    {
        beat_steps = -std::ceil(mouse_wheel_move);
        mouse_wheel_move += beat_steps;

        const float beat_floor = std::floorf(song_ui_elem.get_beat() * get_ticks_per_beat()) / get_ticks_per_beat();
        new_beat = (std::abs(beat_floor - song_ui_elem.get_beat()) < compose_tab::beat_lock_threshold) 
            ? beat_floor - (1.0f / get_ticks_per_beat()) * beat_steps 
            : beat_floor - (1.0f / get_ticks_per_beat()) * (beat_steps - 1);
    }
    else if (mouse_wheel_move >= 1.0f)
    {
        beat_steps = std::floor(mouse_wheel_move);
        mouse_wheel_move -= beat_steps;

        const float beat_ceil = std::ceilf(song_ui_elem.get_beat() * get_ticks_per_beat()) / get_ticks_per_beat();
        new_beat = (std::abs(beat_ceil - song_ui_elem.get_beat()) < compose_tab::beat_lock_threshold) 
            ? beat_ceil + (1.0f / get_ticks_per_beat()) * beat_steps 
            : beat_ceil + (1.0f / get_ticks_per_beat()) * (beat_steps - 1);
    }

    song_ui_elem.set_beat(new_beat);
    return true;
}

void compose_tab::update_element([[maybe_unused]] float dt)
{
    if (song_ui_elem.get_music().IsPlaying() && song_ui_elem.get_prev_beat().has_value())
    {
        const float curr_beat = song_ui_elem.get_beat();
        for (const auto& [_, key_hit_objs] : compose_info.hit_objs)
        {
            auto it = key_hit_objs.upper_bound(curr_beat);
            if (it == key_hit_objs.begin())
                continue;

            it--;
            if (song_ui_elem.get_prev_beat().value() < it->first && it->first <= curr_beat)
                compose_info.hitsound.Play();
            if (it->second.duration > 0.0f && song_ui_elem.get_prev_beat().value() < it->first + it->second.duration && it->first + it->second.duration <= curr_beat)
                compose_info.hitsound.Play();
        }
    }

    beat_snap_button_rect.ref.border.value().opt_color.value() = beat_snap_button_color.get().to_color();
    beat_snap_button_rect.ref.border.value().val = beat_snap_button_outline.get();

    key_lock_button_rect.ref.border.value().opt_color.value() = key_lock_button_color.get().to_color();
    key_lock_button_rect.ref.border.value().val = key_lock_button_outline.get();

    std::get<kee::dims>(tick_l_img.ref.dimensions).w.val = tick_l_button_scale.get();
    std::get<kee::dims>(tick_l_img.ref.dimensions).h.val = tick_l_button_scale.get();
    std::get<kee::dims>(tick_r_img.ref.dimensions).w.val = tick_r_button_scale.get();
    std::get<kee::dims>(tick_r_img.ref.dimensions).h.val = tick_r_button_scale.get();

    for (std::size_t i = 0; i < compose_tab_info::tick_freq_count; i++)
        tick_frame_texts[i].ref.set_opt_color(tick_frame_text_colors[i].get().get().to_color());

    const raylib::Color tick_l_img_color = (compose_info.tick_freq_idx != 0) ? tick_l_button_color.get().to_color() : raylib::Color::Blank();
    const raylib::Color tick_r_img_color = (compose_info.tick_freq_idx != compose_tab_info::tick_freq_count - 1) ? tick_r_button_color.get().to_color() : raylib::Color::Blank();
    tick_l_img.ref.set_opt_color(tick_l_img_color);
    tick_r_img.ref.set_opt_color(tick_r_img_color);

    tick_curr_rect.ref.x.val = tick_curr_rect_x.get();
}

void compose_tab::set_tick_freq_idx(std::size_t new_tick_freq_idx)
{
    compose_info.tick_freq_idx = new_tick_freq_idx;
    tick_text.ref.set_string("1 / " + std::to_string(compose_tab_info::tick_freqs[compose_info.tick_freq_idx]));

    const float new_y = static_cast<float>(compose_info.tick_freq_idx * 2 + 1) / (compose_tab_info::tick_freq_count * 2);
    tick_curr_rect_x.set(std::nullopt, new_y, 0.2f, kee::transition_type::exp);
}

} // namespace editor
} // namespace scene
} // namespace kee