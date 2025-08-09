#pragma once

namespace kee {
namespace ui {

template <std::derived_from<kee::ui::base> T, typename... Args>
unsigned int base::add_child_no_id(Args&&... args)
{
    unsigned int id = 0;
    while (children.contains(id))
        id++;

    children[id] = std::make_unique<T>(*this, std::forward<Args>(args)...);
    if (children_z_order_enabled)
        z_order_refs.push_back(base::ref(id, *children.at(id).get()));

    return id;
}

template <std::derived_from<kee::ui::base> T, typename... Args>
void base::add_child_with_id(unsigned int id, Args&&... args)
{
    auto [_, add_success] = children.try_emplace(id, std::make_unique<T>(*this, std::forward<Args>(args)...));
    if (!add_success)
        throw std::invalid_argument("UI Element already contains child with provided ID!");

    if (children_z_order_enabled)
        z_order_refs.push_back(base::ref(id, *children.at(id).get()));
}

template <std::derived_from<kee::ui::base> T, typename... Args>
T base::make_temp_child(Args&&... args) const
{
    return T(*this, std::forward<Args>(args)...);
}

} // namespace ui
} // namespace kee