#include "kee/ui/scene/base.hpp"

namespace kee {
namespace ui {
namespace scene {

window::window()
{
    static constexpr int window_fps = 60;
    //impl.SetConfigFlags(
    //    ConfigFlags::FLAG_WINDOW_TOPMOST |      /* Keep window correctly positioned while fullscreened */
    //    ConfigFlags::FLAG_WINDOW_UNDECORATED |  /* Do not render anything else besides the window screen itself */
    //);
    //impl.Init(raylib::Window::GetWidth(), raylib::Window::GetHeight());

    impl.SetConfigFlags(ConfigFlags::FLAG_WINDOW_HIGHDPI);
    impl.Init(1200, 675);
    impl.SetTargetFPS(window_fps);
}

base::base(const kee::ui::scene::window& window) :
    kee::ui::base(boost::none),
    window(window)
{
    x = pos(pos::type::beg, 0);
    y = pos(pos::type::beg, 0);
    dimensions = dims(
        dim(dim::type::abs, window.impl.GetWidth()),
        dim(dim::type::abs, window.impl.GetHeight())
    );
    centered = false;

    z_order = std::nullopt;
    children_z_order_enabled = true;

    set_color(raylib::Color(0, 0, 0, 0));
}

void base::update_element(float dt)
{
    auto& [w, h] = std::get<kee::ui::dims>(dimensions);
    w.val = window.impl.GetWidth();
    h.val = window.impl.GetHeight();
}

} // namespace scene
} // namespace ui
} // namespace kee