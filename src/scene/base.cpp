#include "kee/scene/base.hpp"

namespace kee {
namespace scene {

window::window()
{
    static constexpr int window_fps = 60;
    impl.SetConfigFlags(
        ConfigFlags::FLAG_WINDOW_TOPMOST |      /* Keep window correctly positioned while fullscreened */
        ConfigFlags::FLAG_WINDOW_UNDECORATED    /* Do not render anything else besides the window screen itself */
    );
    impl.Init(raylib::Window::GetWidth(), raylib::Window::GetHeight());
    impl.SetTargetFPS(window_fps);
}

base::base(const kee::scene::window& window) :
    kee::ui::base(boost::none),
    window(window)
{
    x = pos(pos::type::beg, 0);
    y = pos(pos::type::beg, 0);
    dimensions = dims(
        dim(dim::type::abs, static_cast<float>(window.impl.GetWidth())),
        dim(dim::type::abs, static_cast<float>(window.impl.GetHeight()))
    );
    centered = false;

    z_order = std::nullopt;
    children_z_order_enabled = true;

    set_color(raylib::Color(0, 0, 0, 0));
}

void base::update_element([[maybe_unused]] float dt)
{
    auto& [w, h] = std::get<kee::dims>(dimensions);
    w.val = static_cast<float>(window.impl.GetWidth());
    h.val = static_cast<float>(window.impl.GetHeight());
}

} // namespace scene
} // namespace kee