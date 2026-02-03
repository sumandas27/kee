#pragma once

#include "kee/scene/base.hpp"
#include "kee/ui/image.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/text.hpp"

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
    static constexpr std::size_t loading_circles_count = 9;

    void update_element(float dt) override;

    std::unique_ptr<kee::scene::base>& curr_scene;
    std::unique_ptr<kee::scene::base>& temp_scene;

    kee::transition<float>& bg_alpha;
    std::vector<std::reference_wrapper<kee::transition<kee::color>>> circle_colors;
    std::vector<std::reference_wrapper<kee::transition<float>>> circle_scales;

    kee::ui::handle<kee::ui::rect> bg;
    kee::ui::handle<kee::ui::text> loading_text;
    kee::ui::handle<kee::ui::base> loading_frame;
    std::vector<kee::ui::handle<kee::ui::rect>> loading_circles;

    std::optional<transition_info> info;
    std::size_t next_circle_trns;

    float circle_trns_time;
};

} // namespace scene
} // namespace kee