#include "kee/ui/scrollable.hpp"

#include "kee/game.hpp"

namespace kee {
namespace ui {

scrollbar::scrollbar(
    const kee::ui::required& reqs,
    const kee::pos& x,
    const kee::dim& w,
    float rel_h
) :
    kee::ui::base(reqs, x,
        pos(pos::type::rel, 0.f),
        dims(w, dim(dim::type::rel, 1.f)),
        false
    ),
    thumb_color(add_transition<kee::color>(kee::color(50, 50, 50))),
    thumb_frame(add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_w, 0.25f),
        true
    )),
    thumb(thumb_frame.ref.add_child<kee::ui::rect>(std::nullopt,
        kee::color(50, 50, 50),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.f),
        dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::rel, 1.f / rel_h)
        ),
        true, std::nullopt,
        rect_roundness(rect_roundness::type::rel_w, 0.5f, std::nullopt)
    )),
    thumb_is_hot(false)
{ 
    const raylib::Rectangle thumb_rect = thumb.ref.get_raw_rect();
    if (thumb_rect.height < thumb_rect.width)
    {
        const float new_rel_h = thumb_rect.width / thumb_frame.ref.get_raw_rect().height;
        std::get<kee::dims>(thumb.ref.dimensions).h.val = new_rel_h;
    }

    thumb.ref.y.val = std::get<kee::dims>(thumb.ref.dimensions).h.val / 2.f;
}

void scrollbar::on_element_mouse_move(const raylib::Vector2& mouse_pos, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    const bool is_mouse_on_thumb = thumb.ref.get_raw_rect().CheckCollision(mouse_pos);
    if (rel_h_down_offset.has_value())
    {
        const float thumb_raw_y = (mouse_pos.y - get_raw_rect().y) / get_raw_rect().height - rel_h_down_offset.value() / 2.f;
        const float thumb_rel_cutoff = std::get<kee::dims>(thumb.ref.dimensions).h.val / 2.f;
        thumb.ref.y.val = std::clamp(thumb_raw_y, thumb_rel_cutoff, 1.f - thumb_rel_cutoff);
    }
    else if (!is_mouse_on_thumb && thumb_is_hot)
    {
        thumb_color.set(std::nullopt, kee::color(50, 50, 50), 0.5f, kee::transition_type::exp);
        thumb_is_hot = false; 
    }
    else if (is_mouse_on_thumb && !thumb_is_hot)
    {
        thumb_color.set(std::nullopt, kee::color(40, 40, 40), 0.5f, kee::transition_type::exp);
        thumb_is_hot = true;
    }
}

bool scrollbar::on_element_mouse_down([[maybe_unused]] const raylib::Vector2& mouse_pos, bool is_mouse_l, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    if (!get_raw_rect().CheckCollision(mouse_pos) || !is_mouse_l)
        return false;

    if (!thumb.ref.get_raw_rect().CheckCollision(mouse_pos))
    {
        const float mouse_rel_h = (mouse_pos.y - get_raw_rect().y) / get_raw_rect().height;
        const float thumb_rel_cutoff = std::get<kee::dims>(thumb.ref.dimensions).h.val / 2.f;
        thumb.ref.y.val = std::clamp(mouse_rel_h, thumb_rel_cutoff, 1.f - thumb_rel_cutoff);
    }

    const raylib::Rectangle thumb_rect = thumb.ref.get_raw_rect();
    rel_h_down_offset = (mouse_pos.y - thumb_rect.y) / thumb_rect.height - 0.5f;

    thumb_color.set(kee::color(40, 40, 40));
    thumb_is_hot = true;

    return true;
}

bool scrollbar::on_element_mouse_up([[maybe_unused]] const raylib::Vector2& mouse_pos, bool is_mouse_l, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    if (!rel_h_down_offset.has_value() || !is_mouse_l)
        return false;

    rel_h_down_offset.reset();
    thumb_is_hot = true;

    return true;
}

void scrollbar::update_element([[maybe_unused]] float dt)
{
    thumb.ref.color = thumb_color.get();
}

scrollable::scrollable(
    const kee::ui::required& reqs,
    const kee::pos& x,
    const kee::pos& y,
    const std::variant<kee::dims, kee::border>& dimensions,
    bool centered,
    const kee::pos& scrollbar_x,
    const kee::dim& scrollbar_w,
    const kee::pos& scroll_frame_x,
    const kee::dim& scroll_frame_w
) :
    kee::ui::base(reqs, x, y, dimensions, centered),
    scrollbar_x(scrollbar_x),
    scrollbar_w(scrollbar_w),
    scroll_frame_bg(add_child<kee::ui::rect>(std::nullopt,
        kee::color(50, 50, 50, 100),
        scroll_frame_x,
        pos(pos::type::rel, 0.f),
        dims(
            scroll_frame_w,
            dim(dim::type::rel, 1.f)
        ),
        false, std::nullopt, std::nullopt
    )),
    scroll_frame(scroll_frame_bg.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0.f),
        true
    ))
{ }

void scrollable::set_scrollable_rel_h(float rel_h)
{
    if (rel_h > 1.f)
        scrollbar_ui.emplace(add_child<scrollbar>(std::nullopt, scrollbar_x, scrollbar_w, rel_h));
    else
        scrollbar_ui.reset();

    /* TODO: reset scroll frame y */
}

void scrollable::render() const
{
    game_ref.scissor_mode.push(get_raw_rect());
    kee::ui::base::render();
    game_ref.scissor_mode.pop();
}

} // namespace ui
} // namespace kee