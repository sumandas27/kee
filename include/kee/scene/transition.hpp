#pragma once

#include "kee/scene/base.hpp"
#include "kee/ui/image.hpp"
#include "kee/ui/rect.hpp"

namespace kee {
namespace scene {

class transition_info
{
public:
    transition_info();

    float time;
    bool scene_switched;
    bool bg_faded_out;
};

/* TODO: impl loading */

class transition final : public kee::scene::base
{
public:
    transition(kee::game& game, kee::global_assets& assets, std::unique_ptr<kee::scene::base>& curr_scene, std::unique_ptr<kee::scene::base>& temp_scene);

    void start();

private:
    void update_element(float dt) override;

    std::unique_ptr<kee::scene::base>& curr_scene;
    std::unique_ptr<kee::scene::base>& temp_scene;

    kee::transition<float>& bg_alpha;

    kee::ui::handle<kee::ui::rect> bg;

    std::optional<transition_info> info;
};

} // namespace scene
} // namespace kee