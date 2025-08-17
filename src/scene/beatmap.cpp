#include "kee/scene/beatmap.hpp"

#include "kee/ui/rect.hpp"
#include "kee/ui/text.hpp"

namespace kee {
namespace scene {

beatmap::beatmap(const kee::scene::window& window, kee::global_assets& assets) :
    kee::scene::base(window, assets),
    input_tolerance(0.25f),
    approach_beats(2.0f),
    load_time(2.0f),
    max_combo_time(0.25f),
    music_start_offset(0.5f),
    music_bpm(100.0f),
    id_trans_combo_gain(0),
    music("assets/daft-punk-something-about-us/daft-punk-something-about-us.mp3"),
    hitsound("assets/sfx/hitsound.wav"),
    combo_lost_sfx("assets/sfx/combo_lost.wav"),
    combo(0),
    combo_time(0.0f),
    end_beat(0.0f),
    game_time(0.0f)
{
    transitions[id_trans_combo_gain] = std::make_unique<kee::transition<float>>(0.0f);

    id_load_rect = add_child_no_id<kee::ui::rect>(
        raylib::Color(255, 255, 255, 20),
        pos(pos::type::beg, 0),
        pos(pos::type::end, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        std::nullopt, std::nullopt,
        kee::ui::common(false, 1, false)
    );

    id_progress_rect = add_child_no_id<kee::ui::rect>(
        raylib::Color::White(),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 0),
            dim(dim::type::abs, 10)
        ),
        std::nullopt, std::nullopt,
        kee::ui::common(false, 0, false)
    );

    id_combo_text = add_child_no_id<kee::ui::text>(
        raylib::Color::White(),
        pos(pos::type::beg, 40),
        pos(pos::type::end, 40),
        ui::text_size(ui::text_size::type::rel_h, 0.1f),
        assets.font_light, "0x", true, 
        kee::ui::common(false, 0, false)
    );

    id_combo_text_bg = add_child_no_id<kee::ui::text>(
        raylib::Color(255, 255, 255, 0),
        pos(pos::type::beg, 40),
        pos(pos::type::end, 40),
        ui::text_size(ui::text_size::type::rel_h, 0.1f),
        assets.font_light, "0x", true, 
        kee::ui::common(false, 0, false)
    );

    id_window_border = add_child_no_id<kee::ui::base>(
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        dims(
            dim(dim::type::rel, 0.9f),
            dim(dim::type::rel, 0.9f)
        ),
        kee::ui::common(true, 0, false)
    );

    id_key_frame = child_at(id_window_border)->add_child_no_id<kee::ui::base>(
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        dims(
            dim(dim::type::aspect, 10),
            dim(dim::type::aspect, 4)
        ),
        kee::ui::common(true, 0, false)
    );

    for (const auto& [id, rel_pos] : kee::key_ui_data)
        child_at(id_window_border)->child_at(id_key_frame)->add_child_with_id<beatmap_key>(id, *this, id, rel_pos);

    music.SetLooping(false);
    music.SetVolume(0.1f);
    hitsound.SetVolume(0.01f);
    combo_lost_sfx.SetVolume(0.05f);

    /**
     * Ideally would have this as class member function, but doing so creates circular references between `beatmap` 
     * and `beatmap_key` that's only resolvable with dirty forward declarations, so I store it locally here instead.
     */
    auto get_key = [&](int keycode) -> beatmap_key& 
    {
        return *dynamic_cast<beatmap_key*>(this->child_at(id_window_border)->child_at(id_key_frame)->child_at(keycode).get());
    };

    /* TODO: replace with some file parser eventually */
    get_key(KeyboardKey::KEY_Q).push(beatmap_hit_object(0.0f, 16.0f));

    //get_key(KeyboardKey::KEY_W).push(beatmap_hit_object(4.0f));
    //get_key(KeyboardKey::KEY_P).push(beatmap_hit_object(8.0f));
    //get_key(KeyboardKey::KEY_O).push(beatmap_hit_object(12.0f));

    for (const auto& [keycode, _] : kee::key_ui_data)
    {
        if (get_key(keycode).get_hit_objects().empty())
            continue;

        const beatmap_hit_object& back = get_key(keycode).get_hit_objects().back();
        if (end_beat < back.beat + back.duration)
            end_beat = back.beat + back.duration;
    }
}

float beatmap::get_beat() const
{
    return (game_time - load_time - music_start_offset) * music_bpm / 60.0f;
}

void beatmap::combo_increment_with_sound()
{
    combo_increment_no_sound();
    hitsound.Play();
}

void beatmap::combo_increment_no_sound()
{
    combo++;
    
    auto& combo_gain = *dynamic_cast<kee::transition<float>*>(transitions.at(id_trans_combo_gain).get());
    combo_gain.set(1.0f, 0.0f, 0.25f, transition_type::lin);
}

void beatmap::combo_lose()
{
    combo = 0;
    combo_lost_sfx.Play();
    
    auto& combo_gain = *dynamic_cast<kee::transition<float>*>(transitions.at(id_trans_combo_gain).get());
    combo_gain.set(0.0f);
}

void beatmap::handle_element_events()
{
    for (int key = raylib::Keyboard::GetKeyPressed(); key != 0; key = raylib::Keyboard::GetKeyPressed())
    {
        if (!child_at(id_window_border)->child_at(id_key_frame)->has_child(key))
            continue;

        auto& key_ui = *dynamic_cast<beatmap_key*>(child_at(id_window_border)->child_at(id_key_frame)->child_at(key).get());
        if (key_ui.get_hit_objects().empty())
            continue;

        beatmap_hit_object& front = key_ui.front();
        const bool is_active = (get_beat() >= front.beat - input_tolerance);
        const bool is_hold_held = (front.duration != 0.0f && front.hold_is_held);
        if (!is_active || is_hold_held)
            continue;

        bool gain_tap_combo = false;
        const bool is_in_tap_range = (std::abs(front.beat - get_beat()) <= input_tolerance);
        const bool is_hold_press_complete = (front.duration != 0.0f && front.hold_press_complete);
        if (is_in_tap_range && !is_hold_press_complete)
        {
            gain_tap_combo = true;
            combo_increment_with_sound();

            if (front.duration == 0.0f)
                key_ui.pop();
        }

        if (front.duration != 0.0f)
        {
            front.hold_is_held = true;
            front.hold_press_complete = true;

            if (!gain_tap_combo)
                front.hold_next_combo = (std::floor(get_beat() + 1.0f) < front.beat + front.duration)
                    ? std::make_optional(std::floor(get_beat() + 1.0f))
                    : std::nullopt;
        }
    }
}

void beatmap::update_element(float dt)
{
    auto& combo_gain = *dynamic_cast<kee::transition<float>*>(transitions.at(id_trans_combo_gain).get());
    auto& combo_text = *dynamic_cast<kee::ui::text*>(child_at(id_combo_text).get());
    auto& combo_text_bg = *dynamic_cast<kee::ui::text*>(child_at(id_combo_text_bg).get());

    combo_text.set_string(std::to_string(combo) + "x");
    combo_text.set_scale(1.0f + 0.1f * combo_gain.get());

    combo_text_bg.set_opt_color(raylib::Color(255, 255, 255, static_cast<unsigned char>(127.5f * combo_gain.get())));
    combo_text_bg.set_string(std::to_string(combo) + "x");
    combo_text_bg.set_scale(1.0f + 0.5f * combo_gain.get());

    game_time += dt;
    if (game_time < load_time)
    {
        auto& [_, h] = std::get<kee::dims>(child_at(id_load_rect)->dimensions);
        h.val = (1.0f - game_time / load_time);
    }
    else if (has_child(id_load_rect))
        remove_child(id_load_rect);

    kee::dim& progress_rect_w = std::get<kee::dims>(child_at(id_progress_rect)->dimensions).w;
    progress_rect_w.val = std::clamp(get_beat() / end_beat, 0.0f, 1.0f);

    if (!music.IsPlaying())
    {
        if (game_time >= load_time)
            music.Play();
    }
    else
        music.Update();
}

beatmap_hit_object::beatmap_hit_object(float beat) :
    beat(beat),
    duration(0.0f)
{ }

beatmap_hit_object::beatmap_hit_object(float beat, float duration) :
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

beatmap_key::beatmap_key(const kee::ui::base::required& reqs, kee::scene::beatmap& beatmap_scene, int key_id, const raylib::Vector2& relative_pos) :
    kee::ui::base(reqs,
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

    set_opt_color(raylib::Color::White());

    transitions[id_trans_combo_lost_alpha] = std::make_unique<kee::transition<float>>(0.0f);

    id_rect = add_child_no_id<kee::ui::rect>(
        raylib::Color::Blank(),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        border(border::type::rel_h, kee::key_border_parent_h),
        ui::rect_outline(ui::rect_outline::type::rel_h, kee::key_border_width, std::nullopt), 
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

    add_child_no_id<kee::ui::text>(
        std::nullopt,
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        ui::text_size(ui::text_size::type::rel_h, 0.5 * (1.0f - 2 * kee::key_border_parent_h)),
        assets.font_light, key_str, false, 
        kee::ui::common(true, 0, false)
    );
}

const std::deque<beatmap_hit_object>& beatmap_key::get_hit_objects() const
{
    return hit_objects;
}

beatmap_hit_object& beatmap_key::front()
{
    return hit_objects.front();
}

void beatmap_key::push(const beatmap_hit_object& object)
{
    if (!hit_objects.empty() && object.beat <= hit_objects.back().beat + hit_objects.back().duration)
        throw std::invalid_argument("A new hit object must be strictly after all other ones in its key!");

    hit_objects.push_back(object);
}

void beatmap_key::pop()
{
    hit_objects.pop_front();
}

void beatmap_key::handle_element_events()
{
    set_opt_color(raylib::Keyboard::IsKeyDown(keycode) ? raylib::Color::Green() : raylib::Color::White());

    kee::transition<float>& combo_lost_alpha = *dynamic_cast<kee::transition<float>*>(transitions.at(id_trans_combo_lost_alpha).get());
    child_at(id_rect)->child_at(id_combo_lost_rect)->set_opt_color(raylib::Color(255, 0, 0, static_cast<unsigned char>(combo_lost_alpha.get())));

    if (hit_objects.empty())
        return;

    beatmap_hit_object& front = hit_objects.front();
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

void beatmap_key::render_element_ahead_children() const
{
    for (const beatmap_hit_object& object : hit_objects)
    {
        if (object.beat + object.duration < beatmap_scene.get_beat())
            continue;

        if (object.beat > beatmap_scene.get_beat() + beatmap_scene.approach_beats)
            break;

        const float start_progress = std::max((object.beat - beatmap_scene.get_beat()) / (2 * beatmap_scene.approach_beats), 0.0f);
        const float end_progress = std::max((object.beat + object.duration - beatmap_scene.get_beat()) / (2 * beatmap_scene.approach_beats), 0.0f);
        const kee::ui::rect hit_obj_rect = make_temp_child<kee::ui::rect>(
            raylib::Color::Blank(),
            pos(pos::type::rel, 0.5),
            pos(pos::type::rel, 0.5),
            border(border::type::rel_h, start_progress + kee::key_border_parent_h),
            ui::rect_outline(ui::rect_outline::type::rel_h_parent, std::max(end_progress - start_progress, kee::key_border_width), std::nullopt),
            std::nullopt,
            kee::ui::common(true, std::nullopt, false)
        );

        hit_obj_rect.render();
    }
}

void beatmap_key::combo_lose()
{
    beatmap_scene.combo_lose();

    kee::transition<float>& combo_lost_alpha = *dynamic_cast<kee::transition<float>*>(transitions.at(id_trans_combo_lost_alpha).get());
    combo_lost_alpha.set(127.5f, 0.0f, 1.0f, transition_type::lin);
}

} // namespace scene
} // namespace kee