#include "kee/ui/rect.hpp"

namespace kee {
namespace ui {

rect::rect(
    kee::ui::base& parent, 
    const std::optional<raylib::Color>& color, 
    kee::ui::pos x, 
    kee::ui::pos y, 
    const std::variant<kee::ui::dims, kee::ui::border>& dims, 
    std::optional<kee::ui::border> border,
    bool centered,
    std::optional<int> z_order,
    bool children_z_order_enabled
) :
    kee::ui::base(parent, x, y, dims, centered, z_order, children_z_order_enabled),
    border(border)
{
    set_color(color);
}

void rect::render_element() const 
{ 
    const raylib::Rectangle rect = get_raw_rect();
    border.has_value()
        ? rect.DrawLines(get_color(), border.value().get_raw_size(rect))
        : rect.Draw(get_color());
}

} // namespace ui
} // namespace kee