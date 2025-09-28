#pragma once

namespace kee {
namespace ui {

template <std::derived_from<kee::ui::base> T, typename... Args>
T& base::add_child(std::optional<int> z_order, Args&&... args)
{
    std::println("ADD CHILD START");
    std::unique_ptr<T> child = std::make_unique<T>(
        kee::ui::base::required(*this, assets), 
        std::forward<Args>(args)...
    );

    T& ref = *child;
    children.emplace(z_order.value_or(0), std::move(child));
    std::println("ADD CHILD END");
    return ref;
}

template <std::derived_from<kee::ui::base> T, typename... Args>
T base::make_temp_child(Args&&... args) const
{
    return T(
        kee::ui::base::required(*this, assets), 
        std::forward<Args>(args)...
    );
}

template <typename T>
kee::transition<T>& base::add_transition(const T& default_val)
{
    std::unique_ptr<kee::transition<T>> transition = std::make_unique<kee::transition<T>>(default_val);

    kee::transition<T>& ref = *transition;
    transitions.emplace_back(std::move(transition));
    return ref;
}

} // namespace ui
} // namespace kee