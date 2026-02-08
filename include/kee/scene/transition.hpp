#pragma once

#include "kee/scene/base.hpp"
#include "kee/ui/image.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/text.hpp"

namespace kee {
namespace scene {

    /* TODO: rename to scene_manager or sm */
class transition final : public kee::scene::base
{
public:
    transition(kee::game& game, kee::global_assets& assets, std::unique_ptr<kee::scene::base>& curr_scene);

    template <std::derived_from<kee::scene::base> T, typename... Args>
    void set_scene(Args&&... args);

private:
    static constexpr std::size_t loading_circles_count = 9;

    void update_element(float dt) override;

    std::unique_ptr<kee::scene::base>& curr_scene;
    std::unique_ptr<kee::scene::base> temp_scene; /* TODO: remove */

    kee::transition<float>& bg_alpha;

    kee::ui::handle<kee::ui::rect> bg;

    std::optional<float> trns_time;
};

} // namespace scene
} // namespace kee

#include "kee/scene/transition.ipp"