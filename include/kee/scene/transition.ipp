#pragma once

namespace kee {
namespace scene {

template <std::derived_from<kee::scene::base> T, typename... Args>
void transition::set_scene(Args&&... args)
{
    trns_time = 0.f;
    bg_alpha.set(std::nullopt, 255.f, 0.25f, kee::transition_type::lin);

    temp_scene = std::make_unique<T>(game_ref, assets, std::forward<Args>(args)...);
}

} // namespace scene
} // namespace kee