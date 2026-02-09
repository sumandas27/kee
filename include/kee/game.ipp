#pragma once

namespace kee {

template <std::derived_from<kee::scene::base> T, typename... Args>
std::unique_ptr<T> game::make_scene(Args&&... args)
{
    return std::make_unique<T>(
        kee::scene::required(*this, assets),
        std::forward<Args>(args)...
    );
}

} // namespace kee