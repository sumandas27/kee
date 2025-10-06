#include "kee/scene/base.hpp"

namespace kee {
namespace scene {

window::window()
{
#ifdef __APPLE__
    static constexpr int window_fps = 60;
#else
    static constexpr int window_fps = 144;
#endif

    impl.SetConfigFlags(
        ConfigFlags::FLAG_BORDERLESS_WINDOWED_MODE |    /* Make window take up the entire screen */
        ConfigFlags::FLAG_WINDOW_UNDECORATED |          /* Remove toolbars when displaying the game */
        ConfigFlags::FLAG_VSYNC_HINT |                  /* Prevent screen tearing */
        ConfigFlags::FLAG_WINDOW_HIGHDPI                /* Correct rendering on Apple machines */
    );

    impl.Init(2560, 1440);
    impl.SetTargetFPS(window_fps);
}

void base::render() const
{
    kee::ui::base::render();

    if (render_priority.has_value())
        render_priority.value().render();
}

bool base::on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, bool ctrl_modifier)
{
    if (render_priority.has_value() && render_priority.value().on_mouse_down(mouse_pos, is_mouse_l, ctrl_modifier))
        return true;

    return kee::ui::base::on_element_mouse_down(mouse_pos, is_mouse_l, ctrl_modifier);
}

bool base::on_element_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, bool ctrl_modifier)
{
    if (render_priority.has_value() && render_priority.value().on_mouse_up(mouse_pos, is_mouse_l, ctrl_modifier))
        return true;

    return kee::ui::base::on_element_mouse_up(mouse_pos, is_mouse_l, ctrl_modifier);
}

base::base(const kee::scene::window& window, kee::global_assets& assets) :
    kee::ui::base(kee::ui::base::required(boost::none, assets)),
    window(window)
{
    x = pos(pos::type::beg, 0);
    y = pos(pos::type::beg, 0);
    dimensions = dims(
        dim(dim::type::abs, static_cast<float>(window.impl.GetWidth())),
        dim(dim::type::abs, static_cast<float>(window.impl.GetHeight()))
    );

    centered = false;
    set_opt_color(raylib::Color::Blank());
}

void base::update_element([[maybe_unused]] float dt)
{
    auto& [w, h] = std::get<kee::dims>(dimensions);
    w.val = static_cast<float>(window.impl.GetWidth());
    h.val = static_cast<float>(window.impl.GetHeight());
}

} // namespace scene
} // namespace kee