#include "kee/ui/key.hpp"

#include "kee/scene/beatmap.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/text.hpp"

namespace kee {
namespace ui {

hit_object::hit_object(float beat) :
    beat(beat),
    duration(0.0f)
{ }

hit_object::hit_object(float beat, float duration) :
    beat(beat),
    duration(duration),
    hold_is_held(false),
    hold_press_complete(false)
{ 
    if (duration == 0.0f)
        throw std::invalid_argument("A hold hit object must have non-zero hold duration!");

    if (std::floor(beat + 1.0f) < beat + duration)
        hold_next_combo = std::floor(beat + 1.0f);
}

key::key(const kee::ui::base& parent, kee::scene::beatmap& beatmap_scene, int key_id, const raylib::Vector2& relative_pos) :
    kee::ui::base(parent,
        pos(pos::type::rel, relative_pos.x),
        pos(pos::type::rel, relative_pos.y),
        dims(
            dim(dim::type::aspect, key_id == KeyboardKey::KEY_SPACE ? 7.0f : 1.0f),
            dim(dim::type::rel, 0.25)
        ),
        kee::ui::common(true, std::nullopt, false)
    ),
    beatmap_scene(beatmap_scene),
    keycode(key_id),
    id_trans_combo_lost_alpha(0),
    combo_lost_time(0.0f)
{
    /* TODO: parent dimension verification ??? */

    set_color(raylib::Color::White());

    transitions[id_trans_combo_lost_alpha] = std::make_unique<kee::transition<float>>(0.0f);

    id_rect = add_child_no_id<kee::ui::rect>(
        std::nullopt,
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        border(border::type::rel_h, 0.05f),
        rect_border(rect_border::type::rel_h, key::border_width), 
        std::nullopt,
        kee::ui::common(true, std::nullopt, true)
    );

    id_combo_lost_rect = child_at(id_rect)->add_child_no_id<kee::ui::rect>(
        raylib::Color(255, 0, 0, 0),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        std::nullopt, std::nullopt, 
        kee::ui::common(false, -1, false)
    );

    const std::string key_str = (key_id != KeyboardKey::KEY_SPACE) 
        ? std::string(1, static_cast<char>(key_id)) 
        : "___";

    child_at(id_rect)->add_child_no_id<kee::ui::text>(
        std::nullopt,
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        text_size(text_size::type::rel_h, 0.5),
        key_str, false, 
        kee::ui::common(true, 0, false)
    );
}

const std::deque<kee::ui::hit_object>& key::get_hit_objects() const
{
    return hit_objects;
}

kee::ui::hit_object& key::front()
{
    return hit_objects.front();
}

void key::push(const kee::ui::hit_object& object)
{
    if (!hit_objects.empty() && object.beat <= hit_objects.back().beat + hit_objects.back().duration)
        throw std::invalid_argument("A new hit object must be strictly after all other ones in its key!");

    hit_objects.push_back(object);
}

void key::pop()
{
    hit_objects.pop_front();
}

void key::update_element([[maybe_unused]] float dt)
{
    set_color(raylib::Keyboard::IsKeyDown(keycode) ? raylib::Color::Green() : raylib::Color::White());

    kee::transition<float>& combo_lost_alpha = *dynamic_cast<kee::transition<float>*>(transitions.at(id_trans_combo_lost_alpha).get());
    child_at(id_rect)->child_at(id_combo_lost_rect)->set_color(raylib::Color(255, 0, 0, static_cast<unsigned char>(combo_lost_alpha.get())));

    if (hit_objects.empty())
        return;

    kee::ui::hit_object& front = hit_objects.front();
    if (front.duration == 0.0f || (!front.hold_press_complete && !front.hold_is_held))
    {
        if (beatmap_scene.get_beat() - front.beat > beatmap_scene.input_tolerance)
        {
            combo_lose();
            if (front.duration == 0.0f)
                pop();
            else
                front.hold_press_complete = true;
        }
    }
    else if (front.hold_is_held && !raylib::Keyboard::IsKeyDown(keycode))
    {
        if (beatmap_scene.get_beat() < front.beat + front.duration - beatmap_scene.input_tolerance)
        {
            combo_lose();
            front.hold_is_held = false;
        }
        else
        {
            beatmap_scene.combo_increment_with_sound();
            pop();
        }
    }
    else if (front.hold_is_held && front.hold_next_combo.has_value() && beatmap_scene.get_beat() >= front.hold_next_combo.value())
    {
        beatmap_scene.combo_increment_no_sound();
        front.hold_next_combo = (front.hold_next_combo.value() + 1.0f < front.beat + front.duration)
            ? std::make_optional(front.hold_next_combo.value() + 1.0f)
            : std::nullopt;
    }
    else if (beatmap_scene.get_beat() - (front.beat + front.duration) > beatmap_scene.input_tolerance)
    {
        if (front.hold_is_held)
            combo_lose();

        pop();
    }
}

void key::render_element() const
{
    for (const kee::ui::hit_object& object : hit_objects)
    {
        if (object.beat + object.duration < beatmap_scene.get_beat())
            continue;

        if (object.beat > beatmap_scene.get_beat() + beatmap_scene.approach_beats)
            break;

        const float start_progress = std::max((object.beat - beatmap_scene.get_beat()) / (2 * beatmap_scene.approach_beats), 0.0f);
        const float end_progress = std::max((object.beat + object.duration - beatmap_scene.get_beat()) / (2 * beatmap_scene.approach_beats), 0.0f);
        const kee::ui::rect hit_obj_rect = child_at(id_rect)->make_temp_child<kee::ui::rect>(
            std::nullopt,
            pos(pos::type::rel, 0.5),
            pos(pos::type::rel, 0.5),
            border(border::type::rel_h, start_progress),
            rect_border(rect_border::type::rel_h_parent, std::max(end_progress - start_progress, key::border_width)),
            std::nullopt,
            kee::ui::common(true, std::nullopt, false)
        );

        hit_obj_rect.render();
    }
}

void key::combo_lose()
{
    beatmap_scene.combo_lose();

    kee::transition<float>& combo_lost_alpha = *dynamic_cast<kee::transition<float>*>(transitions.at(id_trans_combo_lost_alpha).get());
    combo_lost_alpha.set(127.5f, 0.0f, 1.0f, transition_type::lin);
}

key::ui_data::ui_data(int raylib_key, const raylib::Vector2& relative_pos) :
    raylib_key(raylib_key),
    relative_pos(relative_pos)
{ }

} // namespace ui
} // namespace kee