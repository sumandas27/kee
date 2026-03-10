#include "kee/scene/beatmap.hpp"

#include "kee/game.hpp"
#include "kee/scene/menu.hpp"

namespace kee {
namespace scene {

beatmap_hit_object_hold::beatmap_hit_object_hold(float beat, float duration, std::string_view end_hitsound) :
    duration(duration),
    hitsound(end_hitsound),
    is_held(false),
    not_missed(true),
    press_complete(false)
{ 
    if (std::floor(beat + 1.0f) < beat + duration)
        next_combo = std::floor(beat + 1.0f);
}

beatmap_hit_object::beatmap_hit_object(float beat, std::string_view start_hitsound) :
    beat(beat),
    hitsound(start_hitsound)
{ }

beatmap_hit_object::beatmap_hit_object(float beat, std::string_view start_hitsound, float duration, std::string_view end_hitsound) :
    beat(beat),
    hitsound(start_hitsound),
    hold(beatmap_hit_object_hold(beat, duration, end_hitsound))
{ }

float beatmap_hit_object::get_end_beat() const
{
    float res = beat;
    if (hold.has_value())
        res += hold.value().duration;

    return res;
}

beatmap_key::beatmap_key(const kee::ui::required& reqs, kee::scene::beatmap& beatmap_scene, int key_id, const raylib::Vector2& relative_pos, const std::vector<key_decoration>& key_colors) :
    kee::ui::base(reqs,
        pos(pos::type::rel, relative_pos.x),
        pos(pos::type::rel, relative_pos.y),
        dims(
            dim(dim::type::aspect, key_id == KeyboardKey::KEY_SPACE ? 8.0f : 1.0f),
            dim(dim::type::rel, 0.25)
        ),
        true
    ),
    is_down(false),
    beatmap_scene(beatmap_scene),
    combo_lost_alpha(add_transition<float>(0.0f)),
    key_colors(key_colors),
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
        std::nullopt, true, assets.font_light, std::string(), false
    )),
    key_colors_idx(0)
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

void beatmap_key::combo_lose(bool is_miss, float lost_beat)
{
    beatmap_scene.combo_lose(is_miss, lost_beat);
    if (is_miss)
        combo_lost_alpha.set(127.5f, 0.0f, 1.0f, transition_type::lin);
}

beatmap_hit_object& beatmap_key::front()
{
    return hit_objects.front();
}

void beatmap_key::push(const beatmap_hit_object& object)
{
    if (!hit_objects.empty() && object.beat <= hit_objects.back().get_end_beat())
        throw std::invalid_argument("A new hit object must be strictly after all other ones in its key!");

    hit_objects.push_back(object);
}

void beatmap_key::pop()
{
    hit_objects.pop_front();
}

void beatmap_key::update_element([[maybe_unused]] float dt)
{
    if (!is_down)
    {
        while (key_colors_idx < key_colors.size() && beatmap_scene.get_beat() > key_colors[key_colors_idx].end_beat)
            key_colors_idx++;

        if (key_colors_idx >= key_colors.size())
            color = kee::color::white;
        else if (beatmap_scene.get_beat() > key_colors[key_colors_idx].start_beat)
        {
            const float deco_duration = key_colors[key_colors_idx].end_beat - key_colors[key_colors_idx].start_beat;
            const float deco_transition_progress = (beatmap_scene.get_beat() - key_colors[key_colors_idx].start_beat) / deco_duration;
            
            color = kee::transition<kee::color>::calculate(
                key_colors[key_colors_idx].start_color, 
                key_colors[key_colors_idx].end_color, 
                deco_transition_progress, 
                key_colors[key_colors_idx].interpolation
            );
        }
        else if (beatmap_scene.get_beat() < key_colors[key_colors_idx].end_beat)
            color = kee::color::white;
    }
    else
        color = kee::color::green_raylib;

    frame.ref.outline.value().color = color;
    key_text.ref.color = color;

    hit_obj_rects.clear();
    for (const beatmap_hit_object& object : hit_objects)
    {
        if (object.get_end_beat() < beatmap_scene.get_beat())
            continue;

        if (object.beat > beatmap_scene.get_beat() + beatmap_scene.approach_beats)
            break;

        const float start_progress = std::max((object.beat - beatmap_scene.get_beat()) / (2 * beatmap_scene.approach_beats), 0.0f);
        const float end_progress = std::max((object.get_end_beat() - beatmap_scene.get_beat()) / (2 * beatmap_scene.approach_beats), 0.0f);
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
    if (!front.hold.has_value() || (!front.hold.value().press_complete && !front.hold.value().is_held))
    {
        if (beatmap_scene.get_beat() - front.beat > beatmap_scene.beat_forgiveness)
        {
            beatmap_scene.total_combo++;
            combo_lose(true, front.beat);

            if (front.hold.has_value())
            {
                front.hold.value().press_complete = true;
                front.hold.value().not_missed = false;
            }
            else
                pop();
        }
    }
    else if (front.hold.has_value() && front.hold.value().next_combo.has_value() && beatmap_scene.get_beat() >= front.hold.value().next_combo.value())
    {
        if (front.hold.value().is_held)
            beatmap_scene.combo_increment(std::nullopt);
        else
            beatmap_scene.total_combo++;

        front.hold.value().next_combo = (front.hold.value().next_combo.value() + 1.0f < front.get_end_beat())
            ? std::make_optional(front.hold.value().next_combo.value() + 1.0f)
            : std::nullopt;
    }
    else if (front.hold.has_value() && beatmap_scene.get_beat() - front.get_end_beat() > beatmap_scene.beat_forgiveness)
    {
        if (front.hold.value().is_held)
            combo_lose(true, front.get_end_beat());

        beatmap_scene.total_combo++;
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
        std::nullopt, true, assets.font_semi_bold, "PAUSE MENU", false
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
        std::nullopt, true, assets.font_semi_bold, "GO BACK", false
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
        std::nullopt, true, assets.font_semi_bold, "RESTART", false
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
        std::nullopt, true, assets.font_semi_bold, "EXIT", false
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
        std::nullopt, false, assets.font_semi_bold, "BG OPACITY", false
    )),
    game_bg_opacity_text(game_bg_opacity_frame_inner.ref.add_child<kee::ui::text>(std::nullopt,
        beatmap_scene.has_game_bg() ? kee::color::white : kee::color(125, 125, 125),
        pos(pos::type::end, 0),
        pos(pos::type::beg, 0),
        ui::text_size(ui::text_size::type::rel_h, 0.5f),
        std::nullopt, false, assets.font_semi_bold, beatmap_scene.has_game_bg() ? std::format("{}%", static_cast<int>(beatmap_scene.get_bg_opacity().value() / 255.f)) : "--", false
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
                false, false
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
        beatmap_scene.total_attempts += beatmap_scene.session_attempts;
        beatmap_scene.update_performance_json();

        this->game_ref.scene_manager.request_scene_switch([&](){
            return game_ref.make_scene<kee::scene::menu>(false, beatmap_scene.beatmap_dir_path);
        });
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
    if (keycode != KeyboardKey::KEY_ESCAPE)
        return true;

    beatmap_scene.total_attempts += beatmap_scene.session_attempts;
    beatmap_scene.update_performance_json();

    game_ref.scene_manager.request_scene_switch([&](){
        return game_ref.make_scene<kee::scene::menu>(false, beatmap_scene.beatmap_dir_path);
    });

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
    beatmap_scene(beatmap_scene),
    ui_rel_x(add_transition<float>(1)),
    leave_color(add_transition<kee::color>(kee::color::white)),
    restart_color(add_transition<kee::color>(kee::color::white)),
    level_frame(add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(kee::border::type::rel_h, 0.05f),
        true
    )),
    display_outer_frame(level_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 0.f),
        dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::rel, 0.2f)
        ),
        false
    )),
    display_frame(display_outer_frame.ref.add_child<kee::ui::rect>(std::nullopt,
        kee::color(50, 50, 50),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(kee::border::type::rel_w, 0.01f),
        true, std::nullopt,
        ui::rect_roundness(ui::rect_roundness::type::rel_h, 0.2f, std::nullopt)
    )),
    image_frame(display_frame.ref.add_child<kee::ui::rect>(std::nullopt,
        kee::color(10, 10, 10),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        kee::dims(
            dim(dim::type::aspect, static_cast<float>(kee::window_w) / kee::window_h),
            dim(dim::type::rel, 1.f)
        ),
        false, std::nullopt, std::nullopt
    )),
    image(beatmap_scene.level_img.has_value() ?
        std::make_optional(image_frame.ref.add_child<kee::ui::image>(std::nullopt,
            beatmap_scene.level_img.value(), 
            kee::color::white,
            pos(pos::type::rel, 0.5f),
            pos(pos::type::rel, 0.5f),
            border(border::type::abs, 0),
            true, ui::image::display::shrink_to_fit, false, false, 0.0f
        )) :
        std::nullopt
    ),
    text_frame(display_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::beg, 0.f),
        pos(pos::type::rel, 0.f),
        kee::dims(
            dim(dim::type::abs, 0.f),
            dim(dim::type::rel, 1.f)
        ),
        false
    )),
    text_inner_frame(text_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.1f),
        true
    )),
    song_name_text(text_inner_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 0.f),
        ui::text_size(ui::text_size::type::rel_h, 0.6f),
        std::nullopt, false, assets.font_semi_bold, beatmap_scene.song_name, false
    )),
    song_artist_text(text_inner_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(200, 200, 200),
        pos(pos::type::rel, 0.f),
        pos(pos::type::end, 0.f),
        ui::text_size(ui::text_size::type::rel_h, 0.35f),
        std::nullopt, false, assets.font_regular, beatmap_scene.song_artist, false
    )),
    level_name_text(text_inner_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(150, 150, 150),
        pos(pos::type::beg, 0.f),
        pos(pos::type::end, 0.f),
        ui::text_size(ui::text_size::type::rel_h, 0.35f),
        std::nullopt, false, assets.font_italic, " - " + beatmap_scene.mapper + "'s " + beatmap_scene.level_name, false
    )),
    ui_frame(level_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 0.2f),
        dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::rel, 0.6f)
        ),
        false
    )),
    ui_inner_frame(ui_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_w, 0.01f),
        true
    )),
    performance_outer_frame(level_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.25f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::rel, 0.15f),
            dim(dim::type::rel, 0.35f)
        ),
        true
    )),
    performance_frame(performance_outer_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(border::type::rel_h, 0.15f),
        true
    )),
    rating_rect(performance_frame.ref.add_child<kee::ui::rect>(std::nullopt,
        kee::color(120, 120, 120),
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 0.f),
        kee::dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::aspect, 0.32f)
        ),
        false, std::nullopt,
        ui::rect_roundness(ui::rect_roundness::type::rel_h, 0.5f, std::nullopt)
    )),
    rating_frame(rating_rect.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.45f),
        pos(pos::type::rel, 0.5f),
        kee::dims(
            dim(dim::type::abs, 0.f),
            dim(dim::type::rel, 1.f)
        ),
        true
    )),
    rating_star_img(rating_frame.ref.add_child<kee::ui::image>(std::nullopt,
        assets.star_png, kee::color::white,
        pos(pos::type::beg, 0.f),
        pos(pos::type::rel, 0.5f),
        kee::dims(
            dim(dim::type::aspect, 1.f),
            dim(dim::type::rel, 0.8f)
        ),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    rating_text(rating_star_img.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 1.25f),
        pos(pos::type::rel, 0.f),
        ui::text_size(ui::text_size::type::rel_h, 1.f),
        std::nullopt, false, assets.font_semi_bold, "1", false
    )),
    high_score_text_frame(performance_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.f),
        pos(pos::type::end, 0.f),
        kee::dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::abs, 0.f)
        ),
        false
    )),
    high_score_text(high_score_text_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 1.f),
        std::nullopt, true, assets.font_semi_bold, std::string(), false
    )),
    leave_button(level_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.25f),
        pos(pos::type::rel, 0.9f),
        dims(
            dim(dim::type::aspect, 1.f),
            dim(dim::type::rel, 0.1f)
        ),
        true
    )),
    leave_img(leave_button.ref.add_child<kee::ui::image>(std::nullopt,
        beatmap_scene.leave_png, kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(kee::border::type::abs, 0),
        true, ui::image::display::shrink_to_fit, false, false, 0.f
    )),
    leave_text(leave_img.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 4.f / 3.f),
        ui::text_size(ui::text_size::type::rel_h, 1.f / 3.f),
        std::nullopt, true, assets.font_regular, "LEAVE", false
    )),
    restart_button(level_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.75f),
        pos(pos::type::rel, 0.9f),
        dims(
            dim(dim::type::aspect, 1.f),
            dim(dim::type::rel, 0.1f)
        ),
        true
    )),
    restart_img(restart_button.ref.add_child<kee::ui::image>(std::nullopt,
        beatmap_scene.restart_png, kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(kee::border::type::abs, 0),
        true, ui::image::display::shrink_to_fit, false, false, 0.f
    )),
    restart_text(restart_img.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 4.f / 3.f),
        ui::text_size(ui::text_size::type::rel_h, 1.f / 3.f),
        std::nullopt, true, assets.font_regular, "RESTART", false
    )),
    label_frame(ui_inner_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.f),
        dims(
            dim(dim::type::rel, 0.5f),
            dim(dim::type::rel, 1.f)
        ),
        false
    )),
    score_text(label_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 0.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, end_screen::result_text_size),
        std::nullopt, false, assets.font_semi_bold, "Score", true
    )),
    accuracy_text(label_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 1.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, end_screen::result_text_size),
        std::nullopt, false, assets.font_semi_bold, "Accuracy", true
    )),
    missed_text(label_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 2.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, end_screen::result_text_size),
        std::nullopt, false, assets.font_semi_bold, "Missed", true
    )),
    combo_text(label_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 3.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, end_screen::result_text_size),
        std::nullopt, false, assets.font_semi_bold, "Combo", true
    )),
    highest_combo_text(label_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 4.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, end_screen::result_text_size),
        std::nullopt, false, assets.font_semi_bold, "Best Streak", true
    )),
    attempts_text(label_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 5.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, end_screen::result_text_size),
        std::nullopt, false, assets.font_semi_bold, "Attempts", true
    )),
    score_result(label_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::end, 0.f),
        pos(pos::type::rel, 0.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, end_screen::result_text_size),
        std::nullopt, false, assets.font_regular, std::string(), true
    )),
    accuracy_result(label_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::end, 0.f),
        pos(pos::type::rel, 1.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, end_screen::result_text_size),
        std::nullopt, false, assets.font_regular, std::string(), true
    )),
    missed_result(label_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::end, 0.f),
        pos(pos::type::rel, 2.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, end_screen::result_text_size),
        std::nullopt, false, assets.font_regular, std::to_string(beatmap_scene.misses), true
    )),
    combo_result(label_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::end, 0.f),
        pos(pos::type::rel, 3.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, end_screen::result_text_size),
        std::nullopt, false, assets.font_regular, std::string(), true
    )),
    highest_combo_result(label_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::end, 0.f),
        pos(pos::type::rel, 4.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, end_screen::result_text_size),
        std::nullopt, false, assets.font_regular, std::string(), true
    )),
    attempts_result(label_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::end, 0.f),
        pos(pos::type::rel, 5.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, end_screen::result_text_size),
        std::nullopt, false, assets.font_regular, std::string(), true
    )),
    should_reset_flag(false)
{
    const float image_frame_w = image_frame.ref.get_raw_rect().width;
    text_frame.ref.x.val = image_frame_w;
    std::get<kee::dims>(text_frame.ref.dimensions).w.val = display_outer_frame.ref.get_raw_rect().width - image_frame_w;
    level_name_text.ref.x.val = song_artist_text.ref.get_raw_rect().width;

    rating_star_img.ref.x.val = rating_star_img.ref.get_raw_rect().width / 2;
    std::get<kee::dims>(rating_frame.ref.dimensions).w.val = rating_star_img.ref.get_raw_rect().width + rating_text.ref.get_raw_rect().width;
    std::get<kee::dims>(high_score_text_frame.ref.dimensions).h.val = performance_frame.ref.get_raw_rect().height - rating_rect.ref.get_raw_rect().height;

    if (beatmap_scene.total_combo != beatmap_scene.metadata_total_combo)
        throw std::runtime_error(std::format("Total combo ({}x) does not match metadata total combo ({}x)", beatmap_scene.total_combo, beatmap_scene.metadata_total_combo));

    const unsigned int session_score = beatmap_scene.score.value_or(beatmap::score_fc);
    const unsigned int curr_combo = beatmap_scene.combo + beatmap_scene.prev_accumulated_combo;
    const unsigned int highest_combo = std::max(beatmap_scene.combo, beatmap_scene.prev_highest_combo);

    float accuracy = 100.f;
    if (beatmap_scene.total_combo != 0)
        accuracy *= static_cast<float>(curr_combo) / beatmap_scene.total_combo;

    if (!beatmap_scene.best_performance.has_value() || beatmap_scene.score > beatmap_scene.best_performance.value().high_score)
        beatmap_scene.best_performance.emplace(session_score, beatmap_scene.misses, curr_combo, highest_combo, accuracy);

    beatmap_scene.total_attempts += beatmap_scene.session_attempts;
    beatmap_scene.session_attempts = 0;
    beatmap_scene.update_performance_json();

    const float accuracy_trunc = std::floor(accuracy * 100.f) / 100.f;
    accuracy_result.ref.set_string(std::format("{:.2f}", accuracy_trunc));

    combo_result.ref.set_string(std::format("{}/{}", curr_combo, beatmap_scene.total_combo));
    highest_combo_result.ref.set_string(std::format("{}x", highest_combo));
    attempts_result.ref.set_string(std::to_string(beatmap_scene.total_attempts));

    ui_rel_x.set(std::nullopt, 0.5f, 0.5f, kee::transition_type::exp);

    leave_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->leave_color.set(std::nullopt, kee::color(200, 200, 200), 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->leave_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    leave_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        this->game_ref.scene_manager.request_scene_switch([&](){
            return game_ref.make_scene<kee::scene::menu>(false, this->beatmap_scene.beatmap_dir_path);
        });
    };

    restart_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->restart_color.set(std::nullopt, kee::color(200, 200, 200), 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->restart_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    restart_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        should_reset_flag = true;
    };

    const unsigned int high_score = beatmap_scene.best_performance.value().high_score;
    if (beatmap_scene.best_performance.value().high_score == beatmap::score_fc)
    {
        high_score_text.ref.color = kee::color::gold;
        high_score_text.ref.set_string("FC");
    }
    else
    {
        high_score_text.ref.color = kee::color::white;
        high_score_text.ref.set_string(std::to_string(high_score));
    }

    if (beatmap_scene.score.has_value())
        score_result.ref.set_string(std::to_string(beatmap_scene.score.value()));
    else
    {
        score_result.ref.font = assets.font_semi_bold;
        score_result.ref.color = kee::color::gold;
        score_result.ref.set_string("FC");
    }

    take_keyboard_capture();
}

end_screen::~end_screen()
{
    release_keyboard_capture();
}

bool end_screen::should_reset() const
{
    return should_reset_flag;
}

bool end_screen::on_element_key_down([[maybe_unused]] int keycode, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    if (keycode == KeyboardKey::KEY_ESCAPE)
        game_ref.scene_manager.request_scene_switch([&](){
            return game_ref.make_scene<kee::scene::menu>(false, beatmap_scene.beatmap_dir_path);
        });

    return true;
}

bool end_screen::on_element_key_up([[maybe_unused]] int keycode, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    return true;
}

void end_screen::update_element([[maybe_unused]] float dt)
{
    x.val = ui_rel_x.get();

    leave_img.ref.color = leave_color.get();
    leave_text.ref.color = leave_color.get();

    restart_img.ref.color = restart_color.get();
    restart_text.ref.color = restart_color.get();
}

beatmap::beatmap(const kee::scene::required& reqs, beatmap_dir_info&& beatmap_info) :
    kee::scene::base(reqs),
    leave_png("assets/img/leave.png"),
    restart_png("assets/img/restart.png"),
    beat_forgiveness(beatmap_info.beat_forgiveness),
    approach_beats(beatmap_info.approach_beats),
    level_img(beatmap_info.dir_state.has_image
        ? std::make_optional<raylib::Image>((beatmap_info.dir_state.path / beatmap_dir_state::standard_img_filename).string())
        : std::nullopt
    ),
    song_name(beatmap_info.song_name),
    song_artist(beatmap_info.song_artist),
    mapper(beatmap_info.mapper),
    level_name(beatmap_info.level_name),
    metadata_total_combo(beatmap_info.total_combo),
    beatmap_dir_path(beatmap_info.dir_state.path),
    best_performance(beatmap_info.best),
    total_attempts(beatmap_info.attempt_count),
    session_attempts(0),
    keys_json_obj(beatmap_info.keys_json_obj),
    key_color_json_obj(beatmap_info.key_colors_json_obj),
    video_offset(beatmap_info.dir_state.video_dir_info),
    music_start_offset(beatmap_info.song_start_offset),
    music_bpm(beatmap_info.song_bpm),
    combo_gain(add_transition<float>(0.0f)),
    end_fade_out_alpha(add_transition<float>(0.0f)),
    session_attempts_text_alpha(add_transition<float>(0.0f)),
    game_bg_img(beatmap_info.dir_state.has_image
        ? std::make_optional<raylib::Image>((beatmap_info.dir_state.path / beatmap_dir_state::standard_img_filename).string())
        : std::nullopt
    ),
    game_bg([&]() -> std::variant<std::monostate, kee::ui::handle<kee::ui::image>, kee::ui::handle<kee::ui::video_player>>
    {
        if (beatmap_info.dir_state.video_dir_info.has_value())
            return add_child<kee::ui::video_player>(0,
                beatmap_info.dir_state.path / beatmap_dir_state::standard_vid_filename,
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
        std::nullopt, false, assets.font_regular, "100.00", false
    )),
    score_text(performance_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 1),
        std::nullopt, true, assets.font_semi_bold, "0%", false
    )),
    fc_text(performance_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::gold,
        pos(pos::type::end, 0),
        pos(pos::type::beg, 0),
        ui::text_size(ui::text_size::type::rel_h, 1),
        std::nullopt, false, assets.font_semi_bold, "FC", false
    )),
    combo_text(add_child<kee::ui::text>(3,
        kee::color::white,
        pos(pos::type::beg, 40),
        pos(pos::type::end, 40),
        ui::text_size(ui::text_size::type::rel_h, 0.1f),
        std::nullopt, false, assets.font_light, "0x", true
    )),
    combo_text_bg(add_child<kee::ui::text>(2,
        kee::color(255, 255, 255, 0),
        pos(pos::type::beg, 40),
        pos(pos::type::end, 40),
        ui::text_size(ui::text_size::type::rel_h, 0.1f),
        std::nullopt, false, assets.font_light, "0x", true
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
    music((beatmap_info.dir_state.path / beatmap_dir_state::standard_music_filename).string()),
    combo_lost_sfx("assets/sfx/combo_lost.wav")
{
    if (beatmap_info.custom_hitsounds.has_value())
        hitsounds = std::move(beatmap_info.custom_hitsounds.value());
    else
    {
        static const std::filesystem::path default_hitsound_path = "assets/sfx/hitsound_default";
        for (const std::filesystem::directory_entry& hitsound_wav : std::filesystem::directory_iterator(default_hitsound_path))
            hitsounds[hitsound_wav.path().filename().string()] = raylib::Sound(hitsound_wav.path().string());

        hitsounds.at("normal.wav").SetVolume(0.1f);
    }

    reset_level();
}

float beatmap::get_beat() const
{
    const float music_time = load_time_paused.has_value()
        ? game_time - beatmap::load_time
        : music.GetTimePlayed();

    return (music_time - music_start_offset) * music_bpm / 60.0f;
}

void beatmap::combo_increment(const std::optional<std::string>& hitsound_name)
{
    total_combo++;
    combo++;
    combo_gain.set(1.0f, 0.0f, 0.25f, transition_type::lin);

    if (hitsound_name.has_value())
        hitsounds.at(hitsound_name.value()).Play();
}

void beatmap::combo_lose(bool is_miss, float lost_beat)
{
    if (!score.has_value())
    {
        const float percentage = lost_beat / end_beat;
        score = static_cast<unsigned int>(percentage * 100.f);

        /**
         * The last hit object of any level marks its end, missing it would make score
         * 100 technically without this clamp. The map is not FC'd if combo is lost at
         * any point.
         */
        if (score == beatmap::score_fc)
            score = 99;
    }

    prev_accumulated_combo += combo;
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

void beatmap::update_performance_json()
{
    boost::json::object beatmap_json_obj;
    beatmap_json_obj["attempts"] = total_attempts;
    
    if (best_performance.has_value())
    {
        boost::json::object performance_json_obj;
        performance_json_obj["high_score"] = best_performance.value().high_score;
        performance_json_obj["misses"] = best_performance.value().misses;
        performance_json_obj["combo"] = best_performance.value().combo;
        performance_json_obj["best_streak"] = best_performance.value().best_streak;
        performance_json_obj["acc"] = best_performance.value().acc;

        beatmap_json_obj["best"] = performance_json_obj;
    }
    else
        beatmap_json_obj["best"] = nullptr;

    std::ofstream perf_out(beatmap_dir_path / beatmap_dir_state::standard_performance_filename);
    perf_out << boost::json::serialize(beatmap_json_obj);
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
    end_screen_ui.reset();

    end_fade_out_alpha.set(0.f);
    end_fade_out.reset();
    
    keys.clear();
    for (const auto& [id, rel_pos] : kee::key_ui_data)
    {
        const std::string key_str = std::string(1, static_cast<char>(id));
        const std::vector<key_decoration> key_decos = (key_color_json_obj.has_value())
            ? beatmap_dir_info::get_key_decorations(key_color_json_obj.value().at(key_str).as_array())
            : std::vector<key_decoration>();

        keys.emplace(id, key_frame.ref.add_child<beatmap_key>(std::nullopt, *this, id, rel_pos, key_decos));

        const boost::json::array& key_hit_objs = keys_json_obj.at(key_str).as_array();
        for (const boost::json::value& key_hit_obj : key_hit_objs)
        {
            const boost::json::object& key_hit_obj_json = key_hit_obj.as_object();
            const float beat = static_cast<float>(key_hit_obj_json.at("beat").as_double());
            const std::string start_hitsound = static_cast<std::string>(key_hit_obj_json.at("hitsound").as_string());

            float duration = 0.f;
            if (!key_hit_obj_json.at("hold").is_null())
            {
                const boost::json::object& hold_obj = key_hit_obj_json.at("hold").as_object();
                const std::string end_hitsound = static_cast<std::string>(hold_obj.at("hitsound").as_string());
                
                duration = static_cast<float>(hold_obj.at("duration").as_double());
                keys.at(id).ref.push(beatmap_hit_object(beat, start_hitsound, duration, end_hitsound));
            }
            else
                keys.at(id).ref.push(beatmap_hit_object(beat, start_hitsound));
        }

        if (keys.at(id).ref.get_hit_objects().empty())
            continue;

        const beatmap_hit_object& back = keys.at(id).ref.get_hit_objects().back();
        if (end_beat < back.get_end_beat())
            end_beat = back.get_end_beat();
    }

    music.SetLooping(false);
    music.SetVolume(0.1f);
    music.Seek(0.0f);
    music.Pause();

    combo_lost_sfx.SetVolume(0.05f);

    score.reset();
    total_combo = 0;
    prev_accumulated_combo = 0;
    prev_highest_combo = 0;
    combo = 0;
    misses = 0;

    session_attempts++;
    session_attempts_text_alpha.set(255.f);
    session_attempts_text.emplace(add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.15f),
        ui::text_size(ui::text_size::type::rel_h, 0.07f),
        std::nullopt, true, assets.font_italic, std::format("Attempt {}", session_attempts), true
    ));

    has_attempt_text_fade_out_started = false;

    load_time_paused = false;
    time_till_end_screen.reset();
    game_time = 0.f;
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

    keys.at(keycode).ref.is_down = true;
    if (keys.at(keycode).ref.get_hit_objects().empty())
        return true;

    beatmap_hit_object& front = keys.at(keycode).ref.front();
    const bool is_active = (get_beat() >= front.beat - beat_forgiveness);
    const bool is_hold_held = (front.hold.has_value() && front.hold.value().is_held);
    if (!is_active || is_hold_held)
        return true;

    const bool is_in_tap_range = (std::abs(front.beat - get_beat()) <= beat_forgiveness);
    const bool is_hold_press_complete = (front.hold.has_value() && front.hold.value().press_complete);
    if (is_in_tap_range && !is_hold_press_complete)
    {
        combo_increment(front.hitsound);

        if (!front.hold.has_value())
            keys.at(keycode).ref.pop();
    }

    if (front.hold.has_value())
    {
        front.hold.value().is_held = true;
        front.hold.value().press_complete = true;
    }

    return true;
}

bool beatmap::on_element_key_up(int keycode, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    if (!keys.contains(keycode))
        return true;

    keys.at(keycode).ref.is_down = false;
    if (keys.at(keycode).ref.get_hit_objects().empty())
        return true;

    beatmap_hit_object& front = keys.at(keycode).ref.front();
    if (!front.hold.has_value() || !front.hold.value().is_held)
        return true;
    
    if (get_beat() < front.get_end_beat() - beat_forgiveness)
    {
        keys.at(keycode).ref.combo_lose(front.hold.value().not_missed, get_beat());
        if (front.hold.value().not_missed)
            front.hold.value().not_missed = false;
        
        front.hold.value().is_held = false;
    }
    else
    {
        if (front.hold.value().next_combo.has_value() && front.hold.value().next_combo.value() < front.get_end_beat())
        {
            const unsigned int combo_to_add = static_cast<unsigned int>(std::ceil(front.get_end_beat()) - front.hold.value().next_combo.value());
            total_combo += combo_to_add;
            combo += combo_to_add;
        }    

        combo_increment(front.hold.value().hitsound);
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
        if (game_time >= beatmap::load_time)
        {
            music.Play();
            load_time_paused.reset();
        }
    }
    else
        music.Update();

    if (game_time >= beatmap::attempts_text_full_alpha_time && !has_attempt_text_fade_out_started)
    {
        session_attempts_text_alpha.set(std::nullopt, 0.f, beatmap::attempts_text_fade_out_time, kee::transition_type::lin);
        has_attempt_text_fade_out_started = true;
    }
    else if (game_time >= beatmap::attempts_text_full_alpha_time + beatmap::attempts_text_fade_out_time && session_attempts_text.has_value())
        session_attempts_text.reset();

    if (session_attempts_text.has_value())
        session_attempts_text.value().ref.color.a = static_cast<unsigned char>(session_attempts_text_alpha.get());

    if (auto* video_player_ptr = std::get_if<kee::ui::handle<kee::ui::video_player>>(&game_bg))
    {
        const double video_time = !load_time_paused.has_value() ? music.GetTimePlayed() : 0;
        
        assert(video_offset.has_value());
        video_player_ptr->ref.set_time(video_time - video_offset.value());
    }

    if (pause_menu_ui.has_value() && pause_menu_ui.value().ref.destruct_else_restart().has_value())
    {
        if (pause_menu_ui.value().ref.destruct_else_restart().value())
        {
            pause_menu_ui.reset();
            for (auto& [keycode, key_ui] : keys)
            {
                const bool is_key_really_down = raylib::Keyboard::IsKeyDown(keycode);
                if (key_ui.ref.is_down != is_key_really_down)
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

    if (end_screen_ui.has_value() && end_screen_ui.value().ref.should_reset())
        reset_level();

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

    /* TODO: dont think this works with bpm switches */
    const float curr_progress = std::clamp(get_beat() / end_beat, 0.0f, 1.0f);
    std::get<kee::dims>(progress_rect.ref.dimensions).w.val = curr_progress;

    const unsigned int curr_score = static_cast<int>(curr_progress * 100);
    const std::string score_str = score.has_value()
        ? std::format("{}/{}%", score.value(), curr_score)
        : std::format("{}%", curr_score);

    score_text.ref.set_string(score_str);

    float accuracy = 100.f;
    if (total_combo != 0)
        accuracy *= static_cast<float>(combo + prev_accumulated_combo) / total_combo;

    const float accuracy_trunc = std::floor(accuracy * 100.f) / 100.f;
    accuracy_text.ref.set_string(std::format("{:.2f}", accuracy_trunc));

    if (misses == 0)
    {
        fc_text.ref.set_string("FC");

        fc_text.ref.color = kee::color::gold;
        fc_text.ref.font = assets.font_semi_bold;
    }
    else
    {
        const std::string fc_str = (misses <= 99) ? std::format("{}x", misses) : ">99x";
        fc_text.ref.set_string(fc_str);

        fc_text.ref.color = kee::color::red_raylib;
        fc_text.ref.font = assets.font_regular;
    }

    combo_text.ref.set_string(std::to_string(combo) + "x");
    combo_text.ref.set_text_size_val(0.08f + 0.02f * combo_gain.get());

    combo_text_bg.ref.color = kee::color(255, 255, 255, 127.5f * combo_gain.get());
    combo_text_bg.ref.set_string(std::to_string(combo) + "x");
    combo_text_bg.ref.set_text_size_val(0.08f + 0.04f * combo_gain.get());

    if (load_rect.has_value())
    {
        const float load_rect_rel_h = 1 - game_time / beatmap::load_time;
        if (load_rect_rel_h > 0.0f)
            std::get<kee::dims>(load_rect.value().ref.dimensions).h.val = load_rect_rel_h;
        else
            load_rect.reset();
    }
}

} // namespace scene
} // namespace kee