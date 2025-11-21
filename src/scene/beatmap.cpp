#include "kee/scene/beatmap.hpp"

#include "kee/game.hpp"

namespace kee {
namespace scene {

beatmap_hit_object::beatmap_hit_object(float beat, float duration) :
    beat(beat),
    duration(duration),
    hold_is_held(false),
    hold_not_missed(true),
    hold_press_complete(false)
{ 
    if (std::floor(beat + 1.0f) < beat + duration)
        hold_next_combo = std::floor(beat + 1.0f);
}

beatmap_key::beatmap_key(const kee::ui::required& reqs, kee::scene::beatmap& beatmap_scene, int key_id, const raylib::Vector2& relative_pos) :
    kee::ui::base(reqs,
        pos(pos::type::rel, relative_pos.x),
        pos(pos::type::rel, relative_pos.y),
        dims(
            dim(dim::type::aspect, key_id == KeyboardKey::KEY_SPACE ? 8.0f : 1.0f),
            dim(dim::type::rel, 0.25)
        ),
        true
    ),
    beatmap_scene(beatmap_scene),
    combo_lost_alpha(add_transition<float>(0.0f)),
    frame(add_child<kee::ui::rect>(-1,
        raylib::Color::Blank(),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        border(border::type::rel_h, kee::key_border_parent_h),
        true,
        ui::rect_outline(ui::rect_outline::type::rel_h, kee::key_border_width, std::nullopt), 
        std::nullopt
    )),
    frame_combo_lost(add_child<kee::ui::rect>(0,
        raylib::Color(255, 0, 0, 0),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        border(border::type::rel_h, kee::key_border_parent_h),
        true, std::nullopt, std::nullopt
    )),
    key_text(add_child<kee::ui::text>(-1,
        std::nullopt,
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        ui::text_size(ui::text_size::type::rel_h, 0.5 * (1.0f - 2 * kee::key_border_parent_h)),
        true, assets.font_light, std::string(), false
    )),
    combo_lost_time(0.0f)
{
    set_opt_color(raylib::Color::White());

    const std::string key_str = (key_id != KeyboardKey::KEY_SPACE) 
        ? std::string(1, static_cast<char>(key_id)) 
        : "___";

    key_text.ref.set_string(key_str);
}

const std::deque<beatmap_hit_object>& beatmap_key::get_hit_objects() const
{
    return hit_objects;
}

void beatmap_key::combo_lose(bool is_miss)
{
    beatmap_scene.combo_lose(is_miss);
    if (is_miss)
        combo_lost_alpha.set(127.5f, 0.0f, 1.0f, transition_type::lin);
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

void beatmap_key::update_element([[maybe_unused]] float dt)
{
    hit_obj_rects.clear();
    for (const beatmap_hit_object& object : hit_objects)
    {
        if (object.beat + object.duration < beatmap_scene.get_beat())
            continue;

        if (object.beat > beatmap_scene.get_beat() + beatmap_scene.approach_beats)
            break;

        const float start_progress = std::max((object.beat - beatmap_scene.get_beat()) / (2 * beatmap_scene.approach_beats), 0.0f);
        const float end_progress = std::max((object.beat + object.duration - beatmap_scene.get_beat()) / (2 * beatmap_scene.approach_beats), 0.0f);
        hit_obj_rects.push_back(make_temp_child<kee::ui::rect>(
            raylib::Color::Blank(),
            pos(pos::type::rel, 0.5),
            pos(pos::type::rel, 0.5),
            border(border::type::rel_h, start_progress + kee::key_border_parent_h),
            true,
            ui::rect_outline(ui::rect_outline::type::rel_h_parent, std::max(end_progress - start_progress, kee::key_border_width), std::nullopt),
            std::nullopt
        ));
    }

    frame_combo_lost.ref.set_opt_color(raylib::Color(255, 0, 0, static_cast<unsigned char>(combo_lost_alpha.get())));
    if (hit_objects.empty())
        return;

    beatmap_hit_object& front = hit_objects.front();
    if (front.duration == 0.0f || (!front.hold_press_complete && !front.hold_is_held))
    {
        if (beatmap_scene.get_beat() - front.beat > beatmap_scene.beat_forgiveness)
        {
            beatmap_scene.max_combo++;
            combo_lose(true);

            if (front.duration == 0.0f)
                pop();
            else
                front.hold_press_complete = true;
        }
    }
    else if (front.hold_next_combo.has_value() && beatmap_scene.get_beat() >= front.hold_next_combo.value()) /* get hold intermediate combo */
    {
        if (front.hold_is_held)
            beatmap_scene.combo_increment(false);
        else
            beatmap_scene.max_combo++;

        front.hold_next_combo = (front.hold_next_combo.value() + 1.0f < front.beat + front.duration)
            ? std::make_optional(front.hold_next_combo.value() + 1.0f)
            : std::nullopt;
    }
    else if (beatmap_scene.get_beat() - (front.beat + front.duration) > beatmap_scene.beat_forgiveness)
    {
        if (front.hold_is_held)
            combo_lose(true);

        beatmap_scene.max_combo++;
        pop();
    }
}

void beatmap_key::render_element() const
{
    for (const kee::ui::rect& hit_obj_rect : hit_obj_rects)
        hit_obj_rect.render();
}

pause_menu::pause_menu(const kee::ui::required& reqs, std::optional<bool>& load_time_paused, raylib::Music& music) :
    kee::ui::rect(reqs,
        raylib::Color(255, 255, 255, 20),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        true, std::nullopt, std::nullopt
    ),
    music(music),
    ui_rel_y(add_transition<float>(-0.5f)),
    go_back_color(add_transition<kee::color>(kee::color(0, 200, 0))),
    exit_color(add_transition<kee::color>(kee::color(200, 0, 0))),
    ui_frame(add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(30, 30, 30),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, -0.5f),
        dims(
            dim(dim::type::rel, 0.4f),
            dim(dim::type::rel, 1)
        ),
        true, std::nullopt, std::nullopt
    )),
    pause_menu_text(ui_frame.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.25f),
        ui::text_size(ui::text_size::type::rel_h, 0.06f),
        true, assets.font_semi_bold, "PAUSE MENU", false
    )),
    go_back_button(ui_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.45f),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.05f)
        ),
        false
    )),
    go_back_rect(go_back_button.ref.add_child<kee::ui::rect>(std::nullopt,
        go_back_color.get().to_color(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(kee::border::type::abs, 0),
        true, std::nullopt, std::nullopt
    )),
    go_back_text(go_back_rect.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.7f),
        true, assets.font_semi_bold, "GO BACK", false
    )),
    exit_button(ui_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.05f)
        ),
        false
    )),
    exit_rect(exit_button.ref.add_child<kee::ui::rect>(std::nullopt,
        exit_color.get().to_color(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(kee::border::type::abs, 0),
        true, std::nullopt, std::nullopt
    )),
    exit_text(exit_rect.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.7f),
        true, assets.font_semi_bold, "EXIT", false
    )),
    destruct_flag(false)
{
    /* TODO: make kee colors static const vars use in both beatmap and editor */
    go_back_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->go_back_color.set(std::nullopt, kee::color(0, 150, 0), 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->go_back_color.set(std::nullopt, kee::color(0, 200, 0), 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    go_back_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        this->destruct_flag = true;
    };

    exit_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->exit_color.set(std::nullopt, kee::color(150, 0, 0), 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->exit_color.set(std::nullopt, kee::color(200, 0, 0), 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    exit_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        this->game_ref.queue_game_exit();
    };

    take_keyboard_capture();

    ui_rel_y.set(std::nullopt, 0.5f, 0.5f, kee::transition_type::exp);
    if (music.IsPlaying())
        music.Pause();
    else
        load_time_paused = true;
}

bool pause_menu::on_element_key_down([[maybe_unused]] int keycode, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    return true;
}

bool pause_menu::on_element_key_up([[maybe_unused]] int keycode, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    return true;
}

void pause_menu::update_element([[maybe_unused]] float dt)
{
    ui_frame.ref.y.val = ui_rel_y.get();
    go_back_rect.ref.set_opt_color(go_back_color.get().to_color());
    exit_rect.ref.set_opt_color(exit_color.get().to_color());
}

beatmap::beatmap(const kee::scene::window& window, kee::game& game, kee::global_assets& assets, const std::filesystem::path& beatmap_dir_name) :
    beatmap(window, game, assets, beatmap_dir_info(beatmap_dir_name))
{ }

float beatmap::get_beat() const
{
    const float music_time = load_time_paused.has_value()
        ? game_time - load_time
        : music.GetTimePlayed();

    return (music_time - music_start_offset) * music_bpm / 60.0f;
}

void beatmap::combo_increment(bool play_sfx)
{
    max_combo++;
    combo++;
    combo_gain.set(1.0f, 0.0f, 0.25f, transition_type::lin);

    if (play_sfx)
        hitsound.Play();
}

void beatmap::combo_lose(bool is_miss)
{
    prev_total_combo += combo;
    combo = 0;
    combo_gain.set(0.0f);

    if (is_miss)
    {
        misses++;
        combo_lost_sfx.Play();
    }
}

beatmap::beatmap(const kee::scene::window& window, kee::game& game, kee::global_assets& assets, beatmap_dir_info&& beatmap_info) :
    kee::scene::base(window, game, assets),
    combo_gain(add_transition<float>(0.0f)),
    load_rect(add_child<kee::ui::rect>(0,
        raylib::Color(255, 255, 255, 20),
        pos(pos::type::beg, 0),
        pos(pos::type::end, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        false, std::nullopt, std::nullopt
    )),
    progress_bg(add_child<kee::ui::rect>(1,
        raylib::Color(20, 20, 20),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.01f)
        ),
        false, std::nullopt, std::nullopt
    )),
    progress_rect(add_child<kee::ui::rect>(2,
        raylib::Color::White(),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 0),
            dim(dim::type::rel, 0.01f)
        ),
        false, std::nullopt, std::nullopt
    )),
    performance_bg(add_child<kee::ui::base>(1,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.01f),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.1f)
        ),
        false
    )),
    performance_frame(performance_bg.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.25f),
        true
    )),
    accuracy_text(performance_frame.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        ui::text_size(ui::text_size::type::rel_h, 1),
        false, assets.font_regular, "100.00", false
    )),
    fc_text(performance_frame.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::Gold(),
        pos(pos::type::end, 0),
        pos(pos::type::beg, 0),
        ui::text_size(ui::text_size::type::rel_h, 1),
        false, assets.font_semi_bold, "FC", false
    )),
    combo_text(add_child<kee::ui::text>(2,
        raylib::Color::White(),
        pos(pos::type::beg, 40),
        pos(pos::type::end, 40),
        ui::text_size(ui::text_size::type::rel_h, 0.1f),
        false, assets.font_light, "0x", true
    )),
    combo_text_bg(add_child<kee::ui::text>(1,
        raylib::Color(255, 255, 255, 0),
        pos(pos::type::beg, 40),
        pos(pos::type::end, 40),
        ui::text_size(ui::text_size::type::rel_h, 0.1f),
        false, assets.font_light, "0x", true
    )),
    window_border(add_child<kee::ui::base>(1,
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        dims(
            dim(dim::type::rel, 0.9f),
            dim(dim::type::rel, 0.9f)
        ),
        true
    )),
    key_frame(window_border.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        dims(
            dim(dim::type::aspect, 11),
            dim(dim::type::aspect, 4)
        ),
        true
    )),
    beat_forgiveness(beatmap_info.beat_forgiveness),
    approach_beats(beatmap_info.approach_beats),
    max_combo(0),
    load_time(2.0f),
    music_start_offset(beatmap_info.song_start_offset),
    music_bpm(beatmap_info.song_bpm),
    music(std::move(beatmap_info.song)),
    hitsound("assets/sfx/hitsound.wav"),
    combo_lost_sfx("assets/sfx/combo_lost.wav"),
    prev_total_combo(0),
    combo(0),
    misses(0),
    combo_time(0.0f),
    end_beat(0.0f),
    load_time_paused(false),
    game_time(0.0f)
{
    for (const auto& [id, rel_pos] : kee::key_ui_data)
        keys.emplace(id, key_frame.ref.add_child<beatmap_key>(std::nullopt, *this, id, rel_pos));

    music.SetLooping(false);
    music.SetVolume(0.1f);
    hitsound.SetVolume(0.01f);
    combo_lost_sfx.SetVolume(0.05f);

    keys.at(KeyboardKey::KEY_Q).ref.push(beatmap_hit_object(0.f, 32.f));
    /*for (const auto& [keycode, _] : kee::key_ui_data)
    {
        const std::string key_str = std::string(1, static_cast<char>(keycode));
        const boost::json::array& key_hit_objs = beatmap_info.keys_json_obj.at(key_str).as_array();
        
        for (const boost::json::value& key_hit_obj : key_hit_objs)
        {
            const boost::json::object& key_hit_obj_json = key_hit_obj.as_object();
            const float beat = static_cast<float>(key_hit_obj_json.at("beat").as_double());
            const float duration = static_cast<float>(key_hit_obj_json.at("duration").as_double());
            
            keys.at(keycode).ref.push(beatmap_hit_object(beat, duration));
        }
    }*/

    for (const auto& [keycode, _] : kee::key_ui_data)
    {
        if (keys.at(keycode).ref.get_hit_objects().empty())
            continue;

        const beatmap_hit_object& back = keys.at(keycode).ref.get_hit_objects().back();
        if (end_beat < back.beat + back.duration)
            end_beat = back.beat + back.duration;
    }
}

bool beatmap::on_element_key_down(int keycode, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    if (!keys.contains(keycode))
    {
        if (keycode != KeyboardKey::KEY_ESCAPE)
            return true;

        if (!pause_menu_ui.has_value())
            pause_menu_ui.emplace(add_child<pause_menu>(10, load_time_paused, music));

        return true;
    }

    keys.at(keycode).ref.set_opt_color(raylib::Color::Green());
    if (keys.at(keycode).ref.get_hit_objects().empty())
        return true;

    beatmap_hit_object& front = keys.at(keycode).ref.front();
    const bool is_active = (get_beat() >= front.beat - beat_forgiveness);
    const bool is_hold_held = (front.duration != 0.0f && front.hold_is_held);
    if (!is_active || is_hold_held)
        return true;

    bool gain_tap_combo = false;
    const bool is_in_tap_range = (std::abs(front.beat - get_beat()) <= beat_forgiveness);
    const bool is_hold_press_complete = (front.duration != 0.0f && front.hold_press_complete);
    if (is_in_tap_range && !is_hold_press_complete)
    {
        gain_tap_combo = true;
        combo_increment(true);

        if (front.duration == 0.0f)
            keys.at(keycode).ref.pop();
    }

    if (front.duration != 0.0f)
    {
        front.hold_is_held = true;
        front.hold_press_complete = true;
    }

    return true;
}

bool beatmap::on_element_key_up(int keycode, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    if (!keys.contains(keycode))
        return true;

    keys.at(keycode).ref.set_opt_color(raylib::Color::White());
    if (keys.at(keycode).ref.get_hit_objects().empty())
        return true;

    beatmap_hit_object& front = keys.at(keycode).ref.front();
    if (front.duration == 0.0f || !front.hold_is_held)
        return true;
    
    if (get_beat() < front.beat + front.duration - beat_forgiveness)
    {
        keys.at(keycode).ref.combo_lose(front.hold_not_missed);
        if (front.hold_not_missed)
            front.hold_not_missed = false;
        
        front.hold_is_held = false;
    }
    else
    {
        const float front_end_beat = front.beat + front.duration;
        if (front.hold_next_combo.has_value() && front.hold_next_combo.value() < front_end_beat)
        {
            const unsigned int combo_to_add = static_cast<unsigned int>(std::ceil(front_end_beat) - front.hold_next_combo.value());
            max_combo += combo_to_add;
            combo += combo_to_add;
        }    

        combo_increment(true);
        keys.at(keycode).ref.pop();
    }
    
    return true;
}

void beatmap::update_element(float dt)
{
    if (!load_time_paused.has_value() || !load_time_paused.value())
        game_time += dt;
    
    if (load_time_paused.has_value())
    {
        if (game_time >= load_time)
        {
            music.Play();
            load_time_paused.reset();
        }
    }
    else
        music.Update();

    const float accuracy = (max_combo != 0)
        ? 100.f * static_cast<float>(combo + prev_total_combo) / max_combo
        : 100.f;

    const float accuracy_trunc = std::floor(accuracy * 100.f) / 100.f;
    accuracy_text.ref.set_string(std::format("{:.2f}", accuracy_trunc));
    if (accuracy == 100.f)
        accuracy_text.ref.set_opt_color(raylib::Color::White());
    else if (accuracy >= 90.f)
        accuracy_text.ref.set_opt_color(raylib::Color::Green());
    else if (accuracy >= 80.f)
        accuracy_text.ref.set_opt_color(raylib::Color::Blue());
    else if (accuracy >= 70.f)
        accuracy_text.ref.set_opt_color(raylib::Color::Violet());
    else
        accuracy_text.ref.set_opt_color(raylib::Color::Red());

    if (misses == 0)
    {
        fc_text.ref.set_string("FC");
        fc_text.ref.set_opt_color(raylib::Color::Gold());
    }
    else if (misses <= 10)
    {
        const unsigned char g_and_b = static_cast<unsigned char>(255 * (1.f - (misses - 1) / 9.f));
        fc_text.ref.set_string(std::format("{}x", misses));
        fc_text.ref.set_opt_color(raylib::Color(255, g_and_b, g_and_b));
    }
    else
    {
        fc_text.ref.set_string(">10x");
        fc_text.ref.set_opt_color(raylib::Color::Red());
    }

    combo_text.ref.set_string(std::to_string(combo) + "x");
    combo_text.ref.set_scale(1.0f + 0.1f * combo_gain.get());

    combo_text_bg.ref.set_opt_color(raylib::Color(255, 255, 255, static_cast<unsigned char>(127.5f * combo_gain.get())));
    combo_text_bg.ref.set_string(std::to_string(combo) + "x");
    combo_text_bg.ref.set_scale(1.0f + 0.5f * combo_gain.get());

    std::get<kee::dims>(progress_rect.ref.dimensions).w.val = std::clamp(get_beat() / end_beat, 0.0f, 1.0f);
    if (load_rect.has_value())
    {
        const float load_rect_rel_h = 1 - game_time / load_time;
        if (load_rect_rel_h > 0.0f)
            std::get<kee::dims>(load_rect.value().ref.dimensions).h.val = load_rect_rel_h;
        else
            load_rect.reset();
    }
}

} // namespace scene
} // namespace kee