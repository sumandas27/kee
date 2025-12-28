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
        kee::color::blank,
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        border(border::type::rel_h, kee::key_border_parent_h),
        true,
        ui::rect_outline(ui::rect_outline::type::rel_h, kee::key_border_width, color),
        std::nullopt
    )),
    frame_combo_lost(add_child<kee::ui::rect>(0,
        kee::color(255, 0, 0, 0),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        border(border::type::rel_h, kee::key_border_parent_h),
        true, std::nullopt, std::nullopt
    )),
    key_text(add_child<kee::ui::text>(-1,
        color,
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        ui::text_size(ui::text_size::type::rel_h, 0.5 * (1.0f - 2 * kee::key_border_parent_h)),
        true, assets.font_light, std::string(), false
    ))
{
    color = kee::color::white;

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
    frame.ref.outline.value().color = color;
    key_text.ref.color = color;

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
            kee::color::blank,
            pos(pos::type::rel, 0.5),
            pos(pos::type::rel, 0.5),
            border(border::type::rel_h, start_progress + kee::key_border_parent_h),
            true,
            ui::rect_outline(ui::rect_outline::type::abs, 0, color),
            std::nullopt
        ));

        const float rel_h_parent = std::max(end_progress - start_progress, kee::key_border_width);
        hit_obj_rects.back().outline.value().val = get_raw_rect().height * rel_h_parent;
    }

    frame_combo_lost.ref.color = kee::color(255.f, 0.f, 0.f, combo_lost_alpha.get());
    if (hit_objects.empty())
        return;

    beatmap_hit_object& front = hit_objects.front();
    if (front.duration == 0.0f || (!front.hold_press_complete && !front.hold_is_held))
    {
        if (beatmap_scene.get_beat() - front.beat > beatmap_scene.beat_forgiveness)
        {
            beatmap_scene.max_combo++;
            combo_lose(true);

            if (front.duration != 0.0f)
            {
                front.hold_press_complete = true;
                front.hold_not_missed = false;
            }
            else
                pop();
        }
    }
    else if (front.hold_next_combo.has_value() && beatmap_scene.get_beat() >= front.hold_next_combo.value())
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

pause_menu::pause_menu(const kee::ui::required& reqs, beatmap& beatmap_scene) :
    kee::ui::rect(reqs,
        kee::color(255, 255, 255, 20),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        true, std::nullopt, std::nullopt
    ),
    beatmap_scene(beatmap_scene),
    ui_rel_y(add_transition<float>(-0.5f)),
    go_back_color(add_transition<kee::color>(kee::color(0, 200, 0))),
    restart_color(add_transition<kee::color>(kee::color(200, 200, 0))),
    exit_color(add_transition<kee::color>(kee::color(200, 0, 0))),
    ui_frame(add_child<kee::ui::rect>(std::nullopt,
        kee::color(30, 30, 30),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, -0.5f),
        dims(
            dim(dim::type::rel, 0.4f),
            dim(dim::type::rel, 1)
        ),
        true, std::nullopt, std::nullopt
    )),
    pause_menu_text(ui_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.25f),
        ui::text_size(ui::text_size::type::rel_h, 0.06f),
        true, assets.font_semi_bold, "PAUSE MENU", false
    )),
    go_back_button(ui_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.06f)
        ),
        false
    )),
    go_back_rect(go_back_button.ref.add_child<kee::ui::rect>(std::nullopt,
        go_back_color.get(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(kee::border::type::abs, 0),
        true, std::nullopt, std::nullopt
    )),
    go_back_text(go_back_rect.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.7f),
        true, assets.font_semi_bold, "GO BACK", false
    )),
    restart_button(ui_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.56f),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.06f)
        ),
        false
    )),
    restart_rect(restart_button.ref.add_child<kee::ui::rect>(std::nullopt,
        restart_color.get(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(kee::border::type::abs, 0),
        true, std::nullopt, std::nullopt
    )),
    restart_text(restart_rect.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.7f),
        true, assets.font_semi_bold, "RESTART", false
    )),
    exit_button(ui_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.62f),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.06f)
        ),
        false
    )),
    exit_rect(exit_button.ref.add_child<kee::ui::rect>(std::nullopt,
        exit_color.get(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(kee::border::type::abs, 0),
        true, std::nullopt, std::nullopt
    )),
    exit_text(exit_rect.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.7f),
        true, assets.font_semi_bold, "EXIT", false
    )),
    game_bg_opacity_frame(ui_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.68f),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.12f)
        ),
        false
    )),
    game_bg_opacity_frame_inner(game_bg_opacity_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.2f),
        true
    )),
    game_bg_opacity_label(game_bg_opacity_frame_inner.ref.add_child<kee::ui::text>(std::nullopt,
        beatmap_scene.has_game_bg() ? kee::color::white : kee::color(125, 125, 125),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        ui::text_size(ui::text_size::type::rel_h, 0.5f),
        false, assets.font_semi_bold, "BG OPACITY", false
    )),
    game_bg_opacity_text(game_bg_opacity_frame_inner.ref.add_child<kee::ui::text>(std::nullopt,
        beatmap_scene.has_game_bg() ? kee::color::white : kee::color(125, 125, 125),
        pos(pos::type::end, 0),
        pos(pos::type::beg, 0),
        ui::text_size(ui::text_size::type::rel_h, 0.5f),
        false, assets.font_semi_bold, beatmap_scene.has_game_bg() ? std::format("{}%", static_cast<int>(beatmap_scene.get_bg_opacity().value() / 255.f)) : "--", false
    )),
    game_bg_opacity_slider(beatmap_scene.has_game_bg() ?
        std::variant<kee::ui::handle<kee::ui::slider>, kee::ui::handle<kee::ui::rect>>(
            game_bg_opacity_frame_inner.ref.add_child<kee::ui::slider>(std::nullopt,
                pos(pos::type::rel, 0),
                pos(pos::type::rel, 0.85f),
                dims(
                    dim(dim::type::rel, 1),
                    dim(dim::type::rel, 0.15f)
                ),
                false
            )
        ) :
        std::variant<kee::ui::handle<kee::ui::slider>, kee::ui::handle<kee::ui::rect>>(
            game_bg_opacity_frame_inner.ref.add_child<kee::ui::rect>(std::nullopt,
                kee::ui::slider::track_color,
                pos(pos::type::rel, 0),
                pos(pos::type::rel, 0.85f),
                dims(
                    dim(dim::type::rel, 1),
                    dim(dim::type::rel, 0.15f)
                ),
                false,
                std::nullopt,
                ui::rect_roundness(ui::rect_roundness::type::rel_h, 0.5f, std::nullopt)
            )
        )
    )
{
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
        this->destruct_else_restart_flag = true;    
    };

    restart_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->restart_color.set(std::nullopt, kee::color(150, 150, 0), 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->restart_color.set(std::nullopt, kee::color(200, 200, 0), 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    restart_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        this->destruct_else_restart_flag = false;
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

    const std::optional<float> game_bg_opacity = beatmap_scene.get_bg_opacity();
    if (auto* slider_ptr = std::get_if<kee::ui::handle<kee::ui::slider>>(&game_bg_opacity_slider))
        if (game_bg_opacity.has_value())
            slider_ptr->ref.progress = game_bg_opacity.value() / 255.f;

    take_keyboard_capture();
    beatmap_scene.pause();

    ui_rel_y.set(std::nullopt, 0.5f, 0.5f, kee::transition_type::exp);
}

pause_menu::~pause_menu()
{
    beatmap_scene.unpause();
    release_keyboard_capture();
}

std::optional<bool> pause_menu::destruct_else_restart() const
{
    return destruct_else_restart_flag;
}

bool pause_menu::on_element_key_down([[maybe_unused]] int keycode, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    if (keycode == KeyboardKey::KEY_ESCAPE)
        game_ref.queue_game_exit();

    return true;
}

bool pause_menu::on_element_key_up([[maybe_unused]] int keycode, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    return true;
}

void pause_menu::update_element([[maybe_unused]] float dt)
{
    ui_frame.ref.y.val = ui_rel_y.get();
    go_back_rect.ref.color = go_back_color.get();
    restart_rect.ref.color = restart_color.get();
    exit_rect.ref.color = exit_color.get();

    if (auto* slider_ptr = std::get_if<kee::ui::handle<kee::ui::slider>>(&game_bg_opacity_slider))
    {
        beatmap_scene.set_bg_opacity(255 * slider_ptr->ref.progress);
        game_bg_opacity_text.ref.set_string(std::format("{}%", static_cast<int>(slider_ptr->ref.progress * 100)));
    }
}

end_screen::end_screen(const kee::ui::required& reqs, beatmap& beatmap_scene) :
    kee::ui::rect(reqs,
        kee::color(30, 30, 30),
        pos(pos::type::rel, 1.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(kee::border::type::rel_h, 0.08f),
        true, std::nullopt, std::nullopt
    ),
    ui_rel_x(add_transition<float>(1)),
    exit_text_color(add_transition<kee::color>(kee::color::white)),
    rank_text(add_child<kee::ui::text>(std::nullopt,
        kee::color::blank,
        pos(pos::type::rel, 0.25f),
        pos(pos::type::rel, 0.55f),
        ui::text_size(ui::text_size::type::rel_h, 0.15f),
        true, assets.font_semi_bold, std::string(), false
    )),
    rank_misses_text(add_child<kee::ui::text>(std::nullopt,
        kee::color::blank,
        pos(pos::type::rel, 0.25f),
        pos(pos::type::rel, 0.625f),
        ui::text_size(ui::text_size::type::rel_h, 0.05f),
        true, assets.font_semi_bold, std::string(), false
    )),
    ui_frame(add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(kee::border::type::rel_h, 0.1f),
        true
    )),
    song_name(ui_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        ui::text_size(ui::text_size::type::rel_h, 0.08f),
        false, assets.font_semi_bold, beatmap_scene.song_name, false
    )),
    artist_name(ui_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.11f),
        ui::text_size(ui::text_size::type::rel_h, 0.04f),
        false, assets.font_semi_bold, beatmap_scene.song_artist, false
    )),
    level_name(ui_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.17f),
        ui::text_size(ui::text_size::type::rel_h, 0.04f),
        false, assets.font_semi_bold, std::format("{}'s {}", beatmap_scene.mapper, beatmap_scene.level_name), false
    )),
    exit_button(ui_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::end, 0),
        pos(pos::type::end, 0),
        dims(
            dim(dim::type::rel, 0.5f),
            dim(dim::type::rel, 0.1f)
        ),
        false
    )),
    exit_rect(exit_button.ref.add_child<kee::ui::rect>(std::nullopt,
        kee::color(60, 60, 60),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(kee::border::type::abs, 0),
        true, std::nullopt, std::nullopt
    )),
    exit_text(exit_rect.ref.add_child<kee::ui::text>(std::nullopt,
        exit_text_color.get(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.5f),
        true, assets.font_semi_bold, "EXIT", false
    )),
    performance_text(ui_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.75f),
        pos(pos::type::rel, 0.04f),
        ui::text_size(ui::text_size::type::rel_h, 0.08f),
        true, assets.font_semi_bold, "PERFORMANCE", false
    )),
    label_frame(ui_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 0),
            dim(dim::type::rel, 0.8f)
        ),
        false
    )),
    accuracy_text(label_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.3f),
        ui::text_size(ui::text_size::type::rel_h, 0.08f),
        false, assets.font_semi_bold, "ACCURACY", false
    )),
    missed_text(label_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.45f),
        ui::text_size(ui::text_size::type::rel_h, 0.08f),
        false, assets.font_semi_bold, "MISSED", false
    )),
    combo_text(label_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.6f),
        ui::text_size(ui::text_size::type::rel_h, 0.08f),
        false, assets.font_semi_bold, "COMBO", false
    )),
    highest_combo_text(label_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.75f),
        ui::text_size(ui::text_size::type::rel_h, 0.08f),
        false, assets.font_semi_bold, "HIGHEST COMBO", false
    )),
    results_frame(ui_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 1),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 0),
            dim(dim::type::rel, 0.8f)
        ),
        false
    )),
    accuracy_result(results_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::end, 0),
        pos(pos::type::rel, 0.3f),
        ui::text_size(ui::text_size::type::rel_h, 0.08f),
        false, assets.font_regular, std::string(), false
    )),
    missed_result(results_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::end, 0),
        pos(pos::type::rel, 0.45f),
        ui::text_size(ui::text_size::type::rel_h, 0.08f),
        false, assets.font_regular, std::to_string(beatmap_scene.misses), false
    )),
    combo_result(results_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::end, 0),
        pos(pos::type::rel, 0.6f),
        ui::text_size(ui::text_size::type::rel_h, 0.08f),
        false, assets.font_regular, std::string(), false
    )),
    highest_combo_result(results_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::end, 0),
        pos(pos::type::rel, 0.75f),
        ui::text_size(ui::text_size::type::rel_h, 0.08f),
        false, assets.font_regular, std::string(), false
    ))
{
    truncate_name_str(song_name);
    truncate_name_str(artist_name);
    truncate_name_str(level_name);

    const unsigned int curr_combo = beatmap_scene.combo + beatmap_scene.prev_total_combo;
    const unsigned int highest_combo = std::max(beatmap_scene.combo, beatmap_scene.prev_highest_combo);

    float accuracy = 100.f;
    if (beatmap_scene.max_combo != 0)
        accuracy *= static_cast<float>(curr_combo) / beatmap_scene.max_combo;

    const float accuracy_trunc = std::floor(accuracy * 100.f) / 100.f;
    accuracy_result.ref.set_string(std::format("{:.2f}", accuracy_trunc));

    combo_result.ref.set_string(std::format("{}/{}", curr_combo, beatmap_scene.max_combo));
    highest_combo_result.ref.set_string(std::format("{}x", highest_combo));

    ui_rel_x.set(std::nullopt, 0.5f, 0.5f, kee::transition_type::exp);

    exit_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->exit_text_color.set(std::nullopt, kee::color::dark_orange, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->exit_text_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    exit_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        this->game_ref.queue_game_exit();
    };

    if (beatmap_scene.misses == 0)
    {
        rank_text.ref.color = kee::color::gold;
        rank_text.ref.set_string("FC");
    }
    else
    {
        if (beatmap_scene.misses >= 1 && beatmap_scene.misses <= 10)
        {
            rank_text.ref.y.val = 0.525f;
            rank_misses_text.ref.color = kee::color::dark_gray;
            rank_misses_text.ref.set_string(std::format("({}x)", beatmap_scene.misses));
        }

        const unsigned int accuracy_uint = static_cast<unsigned int>(accuracy);
        rank_text.ref.set_string(std::to_string(accuracy_uint));

        if (accuracy >= 90.f)
            rank_text.ref.color = kee::color::green_raylib;
        else if (accuracy >= 80.f)
            rank_text.ref.color = kee::color::blue_raylib;
        else if (accuracy >= 70.f)
            rank_text.ref.color = kee::color::violet;
        else
            rank_text.ref.color = kee::color::red_raylib;
    }

    take_keyboard_capture();
}

bool end_screen::on_element_key_down([[maybe_unused]] int keycode, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    if (keycode == KeyboardKey::KEY_ESCAPE)
        game_ref.queue_game_exit();

    return true;
}

bool end_screen::on_element_key_up([[maybe_unused]] int keycode, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    return true;
}

void end_screen::update_element([[maybe_unused]] float dt)
{
    x.val = ui_rel_x.get();
    exit_text.ref.color = exit_text_color.get();
}

void end_screen::truncate_name_str(kee::ui::handle<kee::ui::text>& name_ui)
{
    if (name_ui.ref.get_raw_rect().width <= 0.45f * ui_frame.ref.get_raw_rect().width)
        return;

    for (std::size_t str_end_char = name_ui.ref.get_string().size(); str_end_char > 0; str_end_char--)
    {
        const std::string new_str = name_ui.ref.get_string().substr(0, str_end_char) + "...";
        name_ui.ref.set_string(new_str);

        if (name_ui.ref.get_raw_rect().width <= 0.45f * ui_frame.ref.get_raw_rect().width)
            return;
    }
}

beatmap::beatmap(kee::game& game, kee::global_assets& assets, const std::filesystem::path& beatmap_dir_name) :
    beatmap(game, assets, beatmap_dir_info(beatmap_dir_name))
{ }

float beatmap::get_beat() const
{
    const float music_time = load_time_paused.has_value()
        ? game_time - load_time
        : music.GetTimePlayed();

    if (music_time > 100.f)
        std::println("{}: {}", load_time_paused.has_value(), music_time);
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
    if (prev_highest_combo < combo)
        prev_highest_combo = combo;

    combo = 0;
    combo_gain.set(0.0f);

    if (is_miss)
    {
        misses++;
        combo_lost_sfx.Play();
    }
}

void beatmap::pause()
{
    if (music.IsPlaying())
        music.Pause();
    else
        load_time_paused = true;
}

void beatmap::unpause()
{
    if (load_time_paused.has_value())
        load_time_paused = false;
    else
        music.Resume();
}

bool beatmap::has_game_bg() const
{
    return !std::holds_alternative<std::monostate>(game_bg);
}

std::optional<float> beatmap::get_bg_opacity() const
{
    std::optional<float> res = std::nullopt;
    std::visit([&](auto&& ui)
    {
        using T = std::decay_t<decltype(ui)>;
        if constexpr (std::is_same_v<T, kee::ui::handle<kee::ui::image>> || std::is_same_v<T, kee::ui::handle<kee::ui::video_player>>)
            res = ui.ref.color.a;
    }, game_bg);

    return res;
}

void beatmap::set_bg_opacity(float opacity)
{
    if (std::holds_alternative<std::monostate>(game_bg))
        return;
    else if (auto* image_ptr = std::get_if<kee::ui::handle<kee::ui::image>>(&game_bg))
        image_ptr->ref.color.a = opacity;
    else if (auto* video_ptr = std::get_if<kee::ui::handle<kee::ui::video_player>>(&game_bg))
        video_ptr->ref.color.a = opacity;
}

beatmap::beatmap(kee::game& game, kee::global_assets& assets, beatmap_dir_info&& beatmap_info) :
    kee::scene::base(game, assets),
    beat_forgiveness(beatmap_info.beat_forgiveness),
    approach_beats(beatmap_info.approach_beats),
    song_name(beatmap_info.song_name),
    song_artist(beatmap_info.song_artist),
    mapper(beatmap_info.mapper),
    level_name(beatmap_info.level_name),
    keys_json_obj(beatmap_info.keys_json_obj),
    load_time(2.0f),
    music_start_offset(beatmap_info.song_start_offset),
    music_bpm(beatmap_info.song_bpm),
    combo_gain(add_transition<float>(0.0f)),
    end_fade_out_alpha(add_transition<float>(0.0f)),
    game_bg_img(beatmap_info.dir_state.has_image
        ? std::make_optional(kee::image_texture(beatmap_info.dir_state.path / beatmap_dir_info::standard_img_filename))
        : std::nullopt
    ),
    game_bg([&]() -> std::variant<std::monostate, kee::ui::handle<kee::ui::image>, kee::ui::handle<kee::ui::video_player>>
    {
        if (beatmap_info.dir_state.has_video)
            return add_child<kee::ui::video_player>(0,
                beatmap_info.dir_state.path / beatmap_dir_info::standard_vid_filename,
                kee::color(255, 255, 255, 255 * kee::game_start_bg_opacity),
                pos(pos::type::rel, 0.5f),
                pos(pos::type::rel, 0.5f),
                border(border::type::abs, 0),
                true
            );
        else if (game_bg_img.has_value())
            return add_child<kee::ui::image>(0,
                game_bg_img.value(),
                kee::color(255, 255, 255, 255 * kee::game_start_bg_opacity),
                pos(pos::type::rel, 0.5f),
                pos(pos::type::rel, 0.5f),
                border(border::type::abs, 0),
                true, ui::image::display::extend_to_fit, false, false, 0.0f
            );
        else
            return std::monostate();
    }()),
    progress_bg(add_child<kee::ui::rect>(2,
        kee::color(20, 20, 20),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.01f)
        ),
        false, std::nullopt, std::nullopt
    )),
    progress_rect(add_child<kee::ui::rect>(3,
        kee::color::white,
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 0),
            dim(dim::type::rel, 0.01f)
        ),
        false, std::nullopt, std::nullopt
    )),
    performance_bg(add_child<kee::ui::base>(2,
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
        kee::color::white,
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        ui::text_size(ui::text_size::type::rel_h, 1),
        false, assets.font_regular, "100.00", false
    )),
    fc_text(performance_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::gold,
        pos(pos::type::end, 0),
        pos(pos::type::beg, 0),
        ui::text_size(ui::text_size::type::rel_h, 1),
        false, assets.font_semi_bold, "FC", false
    )),
    combo_text(add_child<kee::ui::text>(3,
        kee::color::white,
        pos(pos::type::beg, 40),
        pos(pos::type::end, 40),
        ui::text_size(ui::text_size::type::rel_h, 0.1f),
        false, assets.font_light, "0x", true
    )),
    combo_text_bg(add_child<kee::ui::text>(2,
        kee::color(255, 255, 255, 0),
        pos(pos::type::beg, 40),
        pos(pos::type::end, 40),
        ui::text_size(ui::text_size::type::rel_h, 0.1f),
        false, assets.font_light, "0x", true
    )),
    window_border(add_child<kee::ui::base>(2,
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
            dim(dim::type::aspect, kee::key_aspect_w),
            dim(dim::type::aspect, kee::key_aspect_h)
        ),
        true
    )),
    end_beat(0.0f),
    music(std::move(beatmap_info.song)),
    hitsound("assets/sfx/hitsound.wav"),
    combo_lost_sfx("assets/sfx/combo_lost.wav")
{ 
    reset_level();
}

void beatmap::reset_level()
{
    load_rect.emplace(add_child<kee::ui::rect>(1,
        kee::color(255, 255, 255, 20),
        pos(pos::type::beg, 0),
        pos(pos::type::end, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        false, std::nullopt, std::nullopt
    ));

    pause_menu_ui.reset();
    
    keys.clear();
    for (const auto& [id, rel_pos] : kee::key_ui_data)
        keys.emplace(id, key_frame.ref.add_child<beatmap_key>(std::nullopt, *this, id, rel_pos));

    for (const auto& [keycode, _] : kee::key_ui_data)
    {
        const std::string key_str = std::string(1, static_cast<char>(keycode));
        const boost::json::array& key_hit_objs = keys_json_obj.at(key_str).as_array();
        
        for (const boost::json::value& key_hit_obj : key_hit_objs)
        {
            const boost::json::object& key_hit_obj_json = key_hit_obj.as_object();
            const float beat = static_cast<float>(key_hit_obj_json.at("beat").as_double());
            const float duration = static_cast<float>(key_hit_obj_json.at("duration").as_double());
            
            keys.at(keycode).ref.push(beatmap_hit_object(beat, duration));
        }
    }

    for (const auto& [keycode, _] : kee::key_ui_data)
    {
        if (keys.at(keycode).ref.get_hit_objects().empty())
            continue;

        const beatmap_hit_object& back = keys.at(keycode).ref.get_hit_objects().back();
        if (end_beat < back.beat + back.duration)
            end_beat = back.beat + back.duration;
    }

    music.SetLooping(false);
    music.SetVolume(0.1f);
    music.Seek(0.0f);
    music.Pause();

    hitsound.SetVolume(0.01f);
    combo_lost_sfx.SetVolume(0.05f);

    max_combo = 0;
    prev_total_combo = 0;
    prev_highest_combo = 0;
    combo = 0;
    misses = 0;

    load_time_paused = false,
    game_time = 0.0f;
}

bool beatmap::on_element_key_down(int keycode, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    if (!keys.contains(keycode))
    {
        if (keycode != KeyboardKey::KEY_ESCAPE || time_till_end_screen.has_value())
            return true;

        if (!pause_menu_ui.has_value())
            pause_menu_ui.emplace(add_child<pause_menu>(10, *this));

        return true;
    }

    keys.at(keycode).ref.color = kee::color::green_raylib;
    if (keys.at(keycode).ref.get_hit_objects().empty())
        return true;

    beatmap_hit_object& front = keys.at(keycode).ref.front();
    const bool is_active = (get_beat() >= front.beat - beat_forgiveness);
    const bool is_hold_held = (front.duration != 0.0f && front.hold_is_held);
    if (!is_active || is_hold_held)
        return true;

    const bool is_in_tap_range = (std::abs(front.beat - get_beat()) <= beat_forgiveness);
    const bool is_hold_press_complete = (front.duration != 0.0f && front.hold_press_complete);
    if (is_in_tap_range && !is_hold_press_complete)
    {
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

    keys.at(keycode).ref.color = kee::color::white;
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

    if (auto* video_player_ptr = std::get_if<kee::ui::handle<kee::ui::video_player>>(&game_bg))
    {
        const double video_time = !load_time_paused.has_value() ? music.GetTimePlayed() : 0;
        std::println("{}", video_time);
        video_player_ptr->ref.set_time(video_time);
    }

    if (pause_menu_ui.has_value() && pause_menu_ui.value().ref.destruct_else_restart().has_value())
    {
        if (pause_menu_ui.value().ref.destruct_else_restart().value())
        {
            pause_menu_ui.reset();
            for (auto& [keycode, key_ui] : keys)
            {
                const bool is_key_ui_down = (key_ui.ref.color == kee::color::green_raylib);
                const bool is_key_really_down = game_ref.is_key_down(keycode);
                
                if (is_key_really_down != is_key_ui_down)
                {
                    if (is_key_really_down)
                        on_element_key_down(keycode, magic_enum::containers::bitset<kee::mods>());
                    else
                        on_element_key_up(keycode, magic_enum::containers::bitset<kee::mods>());
                }
            }
        }
        else
            reset_level();
    }

    if (!time_till_end_screen.has_value())
    {
        if (get_beat() >= end_beat)
        {
            time_till_end_screen = 2.5f;

            end_fade_out_alpha.set(std::nullopt, 255, 2.5f, kee::transition_type::lin);
            end_fade_out.emplace(add_child<kee::ui::rect>(9,
                kee::color::blank,
                pos(pos::type::rel, 0.5f),
                pos(pos::type::rel, 0.5f),
                kee::border(kee::border::type::abs, -1),
                true, std::nullopt, std::nullopt
            ));
        }
    }
    else if (!end_screen_ui.has_value())
    {
        time_till_end_screen.value() -= dt;
        if (time_till_end_screen.value() <= 0)
            end_screen_ui.emplace(add_child<end_screen>(10, *this));
    }

    if (end_fade_out.has_value())
    {
        const unsigned char a = static_cast<unsigned char>(end_fade_out_alpha.get());
        end_fade_out.value().ref.color = kee::color(10, 10, 10, a);
    }

    float accuracy = 100.f;
    if (max_combo != 0)
        accuracy *= static_cast<float>(combo + prev_total_combo) / max_combo;

    const float accuracy_trunc = std::floor(accuracy * 100.f) / 100.f;

    accuracy_text.ref.set_string(std::format("{:.2f}", accuracy_trunc));
    if (accuracy == 100.f)
        accuracy_text.ref.color = kee::color::white;
    else if (accuracy >= 90.f)
        accuracy_text.ref.color = kee::color::green_raylib;
    else if (accuracy >= 80.f)
        accuracy_text.ref.color = kee::color::blue_raylib;
    else if (accuracy >= 70.f)
        accuracy_text.ref.color = kee::color::violet;
    else
        accuracy_text.ref.color = kee::color::red_raylib;

    if (misses == 0)
    {
        fc_text.ref.set_string("FC");
        fc_text.ref.color = kee::color::gold;
    }
    else if (misses <= 10)
    {
        const float g_and_b = 255 * (1.f - (misses - 1) / 9.f);
        fc_text.ref.set_string(std::format("{}x", misses));
        fc_text.ref.color = kee::color(255, g_and_b, g_and_b);
    }
    else
    {
        fc_text.ref.set_string(">10x");
        fc_text.ref.color = kee::color::red_raylib;
    }

    combo_text.ref.set_string(std::to_string(combo) + "x");
    combo_text.ref.set_scale(1.0f + 0.1f * combo_gain.get());

    combo_text_bg.ref.color = kee::color(255, 255, 255, 127.5f * combo_gain.get());
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