#include "kee/scene/manager.hpp"

namespace kee {
namespace scene {

switch_info::switch_info(std::function<std::unique_ptr<kee::scene::base>()> deferred_switch) :
    time(0.f),
    deferred_switch(deferred_switch)
{ }

manager::manager(const kee::scene::required& reqs, std::unique_ptr<kee::scene::base>& curr_scene) :
    kee::scene::base(reqs),
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

void manager::request_scene_switch(std::function<std::unique_ptr<kee::scene::base>()> deferred_switch_input)
{
    bg_alpha.set(std::nullopt, 255.f, 0.25f, kee::transition_type::lin);
    switch_opt.emplace(deferred_switch_input);
}

void manager::update_element(float dt)
{
    bg.ref.color.a = bg_alpha.get();
    if (!switch_opt.has_value())
        return;

    switch_opt.value().time += dt;
    if (switch_opt.value().fade_in_waited.has_value())
    {
        if (switch_opt.value().fade_in_waited.value())
        {
            bg_alpha.set(std::nullopt, 0.f, 0.25f, kee::transition_type::lin);
            switch_opt.reset();
        }
        else
            switch_opt.value().fade_in_waited = true;
    }
    else if (switch_opt.value().time >= 0.35f)
    {
        curr_scene = switch_opt.value().deferred_switch();
        switch_opt.value().fade_in_waited = false;
    }
}

} // namespace scene
} // namespace kee