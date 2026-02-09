#pragma once

#include "kee/scene/base.hpp"
#include "kee/ui/image.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/text.hpp"

namespace kee {
namespace scene {

class switch_info
{
public:
    switch_info(std::function<std::unique_ptr<kee::scene::base>()> deferred_switch);

    float time;
    std::function<std::unique_ptr<kee::scene::base>()> deferred_switch;

    std::optional<bool> fade_in_waited;
};

class manager final : public kee::scene::base
{
public:
    manager(const kee::scene::required& reqs, std::unique_ptr<kee::scene::base>& curr_scene);

    void request_scene_switch(std::function<std::unique_ptr<kee::scene::base>()> deferred_switch_input);

private:
    static constexpr std::size_t loading_circles_count = 9;

    void update_element(float dt) override;

    std::unique_ptr<kee::scene::base>& curr_scene;

    kee::transition<float>& bg_alpha;
    kee::ui::handle<kee::ui::rect> bg;

    std::optional<switch_info> switch_opt;
};

} // namespace scene
} // namespace kee