#include "kee/ui/rect.hpp"

namespace kee {
namespace ui {

rect_border::rect_border(rect_border::type rect_border_type, float val) :
    rect_border_type(rect_border_type),
    val(val)
{ }

rect_roundness::rect_roundness(rect_roundness::type rect_roundness_type, float val) :
    rect_roundness_type(rect_roundness_type),
    val(val)
{ }

rect::rect(
    const kee::ui::base& parent, 
    const std::optional<raylib::Color>& color, 
    kee::pos x, 
    kee::pos y, 
    const std::variant<kee::dims, kee::border>& dims, 
    std::optional<kee::ui::rect_border> border,
    std::optional<kee::ui::rect_roundness> roundness,
    const kee::ui::common& common
) :
    kee::ui::base(parent, x, y, dims, common),
    border(border),
    roundness(roundness)
{
    set_color(color);
}

/* TODO: use sdf shader implementation */

void rect::render_element() const 
{
    const raylib::Rectangle rect = get_raw_rect();

    float roundness_size;
    if (roundness.has_value())
    {
        switch (roundness.value().rect_roundness_type)
        {
        case rect_roundness::type::abs:
            roundness_size = roundness.value().val;
            break;
        case rect_roundness::type::rel_w:
            roundness_size = rect.width * roundness.value().val;
            break;
        case rect_roundness::type::rel_h:
            roundness_size = rect.height * roundness.value().val;
            break;
        default:
            std::unreachable();
        }
    }
    else
        roundness_size = 0;

    if (border.has_value())
    {
        float rect_border_size;
        switch (border.value().rect_border_type)
        {
        case rect_border::type::abs:
            rect_border_size = border.value().val;
            break;
        case rect_border::type::rel_w:
            rect_border_size = rect.width * border.value().val;
            break;
        case rect_border::type::rel_w_parent:
            rect_border_size = get_raw_rect_parent().width * border.value().val;
            break;
        case rect_border::type::rel_h:
            rect_border_size = rect.height * border.value().val;
            break;
        case rect_border::type::rel_h_parent:
            rect_border_size = get_raw_rect_parent().height * border.value().val;
            break;
        default:
            std::unreachable();
        }

        rect.DrawRoundedLines(roundness_size, 0, rect_border_size, get_color());
    }
    else
        rect.DrawRounded(roundness_size, 0, get_color());
}

} // namespace ui
} // namespace kee