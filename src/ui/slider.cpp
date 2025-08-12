#include "kee/ui/slider.hpp"

#include "kee/ui/rect.hpp"

namespace kee {
namespace ui {

slider::slider(
    const kee::ui::base& parent,
    kee::pos x, 
    kee::pos y, 
    const std::variant<kee::dims, kee::border>& dimensions, 
    bool centered,
    std::optional<int> z_order
) :
    kee::ui::base(parent, x, y, dimensions, kee::ui::common(centered, z_order, true))
{ 
    add_child_no_id<kee::ui::rect>(
        raylib::Color(255, 255, 255, 40),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        std::nullopt, std::nullopt,
        kee::ui::common(false, 1, false)
    );

    id_fill = add_child_no_id<kee::ui::rect>(
        raylib::Color::White(),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 0),
            dim(dim::type::rel, 1)
        ),
        std::nullopt, std::nullopt,
        kee::ui::common(false, 0, false)
    );
}

void slider::update_element([[maybe_unused]] float dt)
{
    auto& fill = *dynamic_cast<kee::ui::rect*>(child_at(id_fill).get());
    auto& [w, _] = std::get<kee::dims>(fill.dimensions);
    w.val = progress;
}

} // namespace ui
} // namespace kee