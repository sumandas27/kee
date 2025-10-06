#pragma once

namespace kee {
namespace ui {

template <std::derived_from<kee::ui::base> T>
handle<T>::handle(handle&& other) noexcept :
    ref(other.ref),
    multimap(other.multimap),
    it(other.it),
    has_moved(false)
{
    other.has_moved = true;
}

template <std::derived_from<kee::ui::base> T>
handle<T>::~handle()
{
    if (!has_moved)
        multimap.erase(it);
}

template <std::derived_from<kee::ui::base> T>
handle<T>::handle(T& ref, std::multimap<int, std::unique_ptr<kee::ui::base>>& multimap, std::multimap<int, std::unique_ptr<kee::ui::base>>::iterator it) :
    ref(ref),
    multimap(multimap),
    it(it),
    has_moved(false)
{ }

template <std::derived_from<kee::ui::base> T>
void handle<T>::change_z_order(int new_z_order)
{
    auto node = multimap.extract(it);
    node.key() = new_z_order;
    it = multimap.insert(std::move(node));
}

template <std::derived_from<kee::ui::base> T, typename... Args>
kee::ui::handle<T> base::add_child(std::optional<int> z_order, Args&&... args)
{
    std::unique_ptr<T> child = std::make_unique<T>(
        kee::ui::base::required(*this, assets), 
        std::forward<Args>(args)...
    );

    T& ref = *child;
    auto it = children->emplace(z_order.value_or(06), std::move(child));
    return kee::ui::handle<T>(ref, *children, it);
}

template <std::derived_from<kee::ui::base> T, typename... Args>
T base::make_temp_child(Args&&... args)
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