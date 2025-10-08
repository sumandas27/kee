#include "kee/ui/slider.hpp"

namespace kee {
namespace ui {

slider::slider(
    const kee::ui::base::required& reqs,
    kee::pos x, 
    kee::pos y, 
    const std::variant<kee::dims, kee::border>& dimensions, 
    bool centered
) :
    kee::ui::base(reqs, x, y, dimensions, centered),
    on_event([]([[maybe_unused]] slider::event slider_event){}),
    progress(0.0f),
    slider_state(mouse_state::off),
    fill_color(add_transition<kee::color>(kee::color::white())),
    thumb_scale(add_transition<float>(1.75f)),
    track(make_temp_child<kee::ui::rect>(
        raylib::Color(255, 255, 255, 40),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        false, std::nullopt, 
        rect_roundness(rect_roundness::type::rel_h, 0.5f, std::nullopt)
    )),
    fill(make_temp_child<kee::ui::rect>(
        raylib::Color::White(),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 0),
            dim(dim::type::rel, 1)
        ),
        false,
        std::nullopt,
        rect_roundness(rect_roundness::type::rel_h, 0.5f, std::nullopt)
    )),
    thumb(fill.make_temp_child<kee::ui::rect>(
        std::nullopt,
        pos(pos::type::rel, 1.0f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 1.0f),
            dim(dim::type::rel, 1.75f)
        ),
        true,
        std::nullopt,
        rect_roundness(rect_roundness::type::rel_h, 0.5f, std::nullopt)
    ))
{ }

bool slider::is_down() const
{
    return slider_state == mouse_state::down;
}

void slider::on_element_mouse_move(const raylib::Vector2& mouse_pos, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    const raylib::Rectangle raw_rect = get_raw_rect();
    const raylib::Rectangle raw_rect_thumb = thumb.get_raw_rect();

    const bool is_mouse_on_rect =
        mouse_pos.x >= raw_rect.x && mouse_pos.x <= raw_rect.x + raw_rect.width &&
        mouse_pos.y >= raw_rect.y && mouse_pos.y <= raw_rect.y + raw_rect.height;

    const bool is_mouse_on_thumb =
        mouse_pos.x >= raw_rect_thumb.x && mouse_pos.x <= raw_rect_thumb.x + raw_rect_thumb.width &&
        mouse_pos.y >= raw_rect_thumb.y && mouse_pos.y <= raw_rect_thumb.y + raw_rect_thumb.height;

    const bool is_mouse_on_slider = is_mouse_on_rect || is_mouse_on_thumb;
    if (slider_state == mouse_state::down)
    {
        const float mouse_progress = (mouse_pos.x - raw_rect.x) / raw_rect.width;
        progress = std::clamp(mouse_progress, 0.0f, 1.0f);
    }
    else if (!is_mouse_on_slider && slider_state != mouse_state::off)
    {
        fill_color.set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
        slider_state = mouse_state::off;
    }
    else if (is_mouse_on_slider && slider_state == mouse_state::off)
    {
        fill_color.set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
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

    on_event(slider::event::on_release);
    thumb_scale.set(std::nullopt, 1.75f, 0.5f, kee::transition_type::exp);
    fill_color.set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
    
    slider_state = mouse_state::hot;
    return true;
}

void slider::update_element([[maybe_unused]] float dt)
{
    fill.set_opt_color(fill_color.get().to_color());
    std::get<kee::dims>(fill.dimensions).w.val = progress;
    std::get<kee::dims>(thumb.dimensions).h.val = thumb_scale.get();
}

void slider::render_element() const
{
    track.render();
    fill.render();
    thumb.render();
}

} // namespace ui
} // namespace kee