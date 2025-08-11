#include "kee/ui/rect.hpp"

namespace kee {
namespace ui {

rect_border::rect_border(rect_border::type rect_border_type, float val) :
    rect_border_type(rect_border_type),
    val(val)
{ }

rect::rect(
    const kee::ui::base& parent, 
    const std::optional<raylib::Color>& color, 
    kee::pos x, 
    kee::pos y, 
    const std::variant<kee::dims, kee::border>& dims, 
    std::optional<const kee::ui::rect_border> border,
    const kee::ui::common& common
) :
    kee::ui::base(parent, x, y, dims, common),
    border(border)
{
    set_color(color);
}

void rect::render_element() const 
{
    const raylib::Rectangle rect = get_raw_rect();
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

        rect.DrawLines(get_color(), rect_border_size);
    }
    else
        rect.Draw(get_color());
}

} // namespace ui
} // namespace kee