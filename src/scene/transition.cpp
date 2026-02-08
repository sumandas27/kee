#include "kee/scene/transition.hpp"

namespace kee {
namespace scene {

transition::transition(kee::game& game, kee::global_assets& assets, std::unique_ptr<kee::scene::base>& curr_scene) :
    kee::scene::base(game, assets),
    curr_scene(curr_scene),
    bg_alpha(add_transition<float>(0.f)),
    bg(add_child<kee::ui::rect>(std::nullopt,
        kee::color::black,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0),
        true, std::nullopt, std::nullopt
    ))
{ }

void transition::update_element(float dt)
{
    bg.ref.color.a = bg_alpha.get();
    if (!trns_time.has_value())
        return;

    trns_time.value() += dt;
    if (trns_time.value() >= 0.25f)
    {
        if (temp_scene != nullptr)
            curr_scene = std::move(temp_scene);
            
        bg_alpha.set(std::nullopt, 0.f, 0.25f, kee::transition_type::lin);
        trns_time.reset();
    }
}

} // namespace scene
} // namespace kee