#include "kee/scene/transition.hpp"

namespace kee {
namespace scene {

transition_info::transition_info() :
    time(0.f),
    scene_switched(false),
    bg_faded_out(false)
{ }

transition::transition(kee::game& game, kee::global_assets& assets, std::unique_ptr<kee::scene::base>& curr_scene, std::unique_ptr<kee::scene::base>& temp_scene) :
    kee::scene::base(game, assets),
    curr_scene(curr_scene),
    temp_scene(temp_scene),
    bg_alpha(add_transition<float>(0.f)),
    bg(add_child<kee::ui::rect>(std::nullopt,
        kee::color::black,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0),
        true, std::nullopt, std::nullopt
    ))
{ }

void transition::start()
{
    info = transition_info();
    bg_alpha.set(std::nullopt, 255.f, 0.25f, kee::transition_type::lin);
}

void transition::update_element(float dt)
{
    bg.ref.color.a = bg_alpha.get();

    if (!info.has_value())
        return;

    info.value().time += dt;
    if (info.value().time >= 1.f && !info.value().scene_switched)
    {
        info.value().scene_switched = true;
        if (temp_scene != nullptr)
            curr_scene = std::move(temp_scene);
    }
    else if (info.value().time >= 2.f && !info.value().bg_faded_out)
    {
        info.value().bg_faded_out = true;
        bg_alpha.set(std::nullopt, 0.f, 0.25f, kee::transition_type::lin);
    }
    else if (info.value().time >= 2.25f)
        info.reset();
}

} // namespace scene
} // namespace kee