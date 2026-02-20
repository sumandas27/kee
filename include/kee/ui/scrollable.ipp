#pragma once

namespace kee {
namespace ui {

template <std::derived_from<kee::ui::base> T, typename... Args>
kee::ui::handle<T> scrollable::add_scrollable_child(std::optional<int> z_order, Args&&... args)
{
    return scroll_frame.ref.add_child<T>(z_order, std::forward<Args>(args)...);
}

} // namespace ui
} // namespace kee