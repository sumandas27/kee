#pragma once

namespace kee {

template <std::derived_from<kee::scene::base> T, typename... Args>
void game::queue_scene_set(Args&&... args)
{
    temp_scene = std::make_unique<T>(*this, assets, std::forward<Args>(args)...);
}

} // namespace kee