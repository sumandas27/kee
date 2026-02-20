#include "kee/ui/slider.hpp"

namespace kee {
namespace ui {

const kee::color slider::track_color = kee::color(255, 255, 255, 40);

slider::slider(
    const kee::ui::required& reqs,
    const kee::pos& x, 
    const kee::pos& y, 
    const std::variant<kee::dims, kee::border>& dimensions, 
    bool centered,
    bool menu_style
) :
    kee::ui::base(reqs, x, y, dimensions, centered),
    on_event([]([[maybe_unused]] slider::event slider_event){}),
    progress(0.0f),
    fill_color(add_transition<kee::color>(kee::color::white)),
    thumb_scale(add_transition<float>(1.75f)),
    track(make_temp_child<kee::ui::rect>(
        slider::track_color,
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        false, std::nullopt, 
        !menu_style 
            ? std::make_optional(rect_roundness(rect_roundness::type::rel_h, 0.5f, std::nullopt))
            : std::nullopt
    )),
    fill(make_temp_child<kee::ui::rect>(
        kee::color::white,
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 0),
            dim(dim::type::rel, 1)
        ),
        false,
        std::nullopt,
        (!menu_style)
            ? std::make_optional(rect_roundness(rect_roundness::type::rel_h, 0.5f, std::nullopt))
            : std::nullopt
    )),
    thumb(fill.make_temp_child<kee::ui::rect>(
        kee::color::white,
        pos(pos::type::rel, 1.0f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 1.0f),
            dim(dim::type::rel, 1.75f)
        ),
        true,
        std::nullopt,
        rect_roundness(rect_roundness::type::rel_h, 0.5f, std::nullopt)
    )),
    slider_state(mouse_state::off),
    menu_style(menu_style)
{ 
    color = menu_style ? kee::color::blank : kee::color::white;
}

bool slider::is_down() const
{
    return slider_state == mouse_state::down;
}

void slider::on_element_mouse_move(const raylib::Vector2& mouse_pos, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    const raylib::Rectangle raw_rect = get_raw_rect();
    const bool is_mouse_on_slider = raw_rect.CheckCollision(mouse_pos) || thumb.get_raw_rect().CheckCollision(mouse_pos);
    
    if (slider_state == mouse_state::down)
    {
        const float mouse_progress = (mouse_pos.x - raw_rect.x) / raw_rect.width;
        progress = std::clamp(mouse_progress, 0.0f, 1.0f);
    }
    else if (!is_mouse_on_slider && slider_state != mouse_state::off)
    {
        fill_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
        slider_state = mouse_state::off;
    }
    else if (is_mouse_on_slider && slider_state == mouse_state::off)
    {
        const kee::color to_color = (!menu_style)
            ? kee::color::dark_orange
            : kee::color(200, 200, 200);

        fill_color.set(std::nullopt, to_color, 0.5f, kee::transition_type::exp);
        slider_state = mouse_state::hot;
    }
}

bool slider::on_element_mouse_down([[maybe_unused]] const raylib::Vector2& mouse_pos, bool is_mouse_l, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    if (slider_state != mouse_state::hot || !is_mouse_l)
        return false;

    on_event(slider::event::on_down);
    thumb_scale.set(std::nullopt, 1.5f, 0.5f, kee::transition_type::exp);

    slider_state = mouse_state::down;
    return true;
}

bool slider::on_element_mouse_up([[maybe_unused]] const raylib::Vector2& mouse_pos, bool is_mouse_l, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    if (slider_state != mouse_state::down || !is_mouse_l)
        return false;

    const kee::color to_color = (!menu_style)
        ? kee::color::dark_orange
        : kee::color(200, 200, 200);

    on_event(slider::event::on_release);
    thumb_scale.set(std::nullopt, 1.75f, 0.5f, kee::transition_type::exp);
    fill_color.set(std::nullopt, to_color, 0.5f, kee::transition_type::exp);
    
    slider_state = mouse_state::hot;
    return true;
}

void slider::update_element([[maybe_unused]] float dt)
{
    std::get<kee::dims>(fill.dimensions).w.val = progress;
    std::get<kee::dims>(thumb.dimensions).h.val = thumb_scale.get();

    fill.color = fill_color.get();
    if (menu_style)
        thumb.color = fill_color.get();

    fill.color.a = color.a;
    thumb.color.a = color.a;
    track.color.a = slider::track_color.a * color.a / 255.f;
}

void slider::render_element() const
{
    track.render();
    fill.render();
    thumb.render();
}

} // namespace ui
} // namespace kee