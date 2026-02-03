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
    )),
    loading_text(bg.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.4f),
        ui::text_size(ui::text_size::type::rel_h, 0.05f),
        std::nullopt, true, assets.font_regular, "Loading...", false
    )),
    loading_frame(bg.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 1.f),
            dim(dim::type::rel, 1.f)
        ),
        true
    )),
    next_circle_trns(0),
    circle_trns_time(0.f)
{ 
    circle_colors.reserve(transition::loading_circles_count);
    loading_circles.reserve(transition::loading_circles_count);

    for (std::size_t i = 0; i < transition::loading_circles_count; i++)
    {
        circle_colors.push_back(add_transition<kee::color>(kee::color::white));
        circle_scales.push_back(add_transition<float>(1.f));

        const float rad = static_cast<float>(i) / transition::loading_circles_count * 2.f * std::numbers::pi_v<float> - std::numbers::pi_v<float> / 2.f;
        const float rel_x = 0.5f + 0.08f * std::cosf(rad);
        const float rel_y = 0.55f + 0.08f * std::sinf(rad);
        
        loading_circles.push_back(loading_frame.ref.add_child<kee::ui::rect>(std::nullopt,
            circle_colors.back().get().get(),
            pos(pos::type::rel, rel_x),
            pos(pos::type::rel, rel_y),
            dims(
                dim(dim::type::aspect, 1.f),
                dim(dim::type::rel, 0.02f)
            ),
            true, std::nullopt,
            ui::rect_roundness(ui::rect_roundness::type::rel_h, 0.5f, std::nullopt)
        ));
    }
}

void transition::start()
{
    info = transition_info();
    bg_alpha.set(std::nullopt, 255.f, 0.25f, kee::transition_type::lin);
}

void transition::update_element(float dt)
{
    circle_trns_time += dt;

    static constexpr float circle_trns_interval = 0.1f;
    static constexpr float circle_trns_total = circle_trns_interval * transition::loading_circles_count;
    if (circle_trns_time >= circle_trns_interval)
    {
        circle_colors[next_circle_trns].get().set(kee::color::white, kee::color::black, circle_trns_total, kee::transition_type::lin);
        circle_scales[next_circle_trns].get().set(1.5f, 1.f, circle_trns_total, kee::transition_type::lin);

        next_circle_trns = (next_circle_trns + 1) % transition::loading_circles_count;
        circle_trns_time -= circle_trns_interval;
    }

    bg.ref.color.a = bg_alpha.get();
    loading_text.ref.color.a = bg_alpha.get();
    for (std::size_t i = 0; i < transition::loading_circles_count; i++)
    {
        std::get<kee::dims>(loading_circles[i].ref.dimensions).h.val = 0.02f * circle_scales[i].get().get();
        loading_circles[i].ref.color = circle_colors[i].get().get();
        loading_circles[i].ref.color.a = bg_alpha.get();
    }

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