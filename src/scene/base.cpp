#include "kee/scene/base.hpp"

namespace kee {
namespace scene {

window::window()
{
    static constexpr int window_fps = 144;
    impl.SetConfigFlags(
        ConfigFlags::FLAG_BORDERLESS_WINDOWED_MODE |    /* Make window take up the entire screen */
        ConfigFlags::FLAG_WINDOW_UNDECORATED |          /* Remove toolbars when displaying the game */
        ConfigFlags::FLAG_VSYNC_HINT                    /* Prevent screen tearing */
    );
    impl.Init(raylib::Window::GetWidth(), raylib::Window::GetHeight());
    //impl.Init();
    impl.SetTargetFPS(window_fps);
}

base::base(const kee::scene::window& window, kee::global_assets& assets) :
    kee::ui::base(
        kee::ui::base::required(boost::none, assets), 
        kee::ui::common(false, std::nullopt, true)
    ),
    window(window)
{
    x = pos(pos::type::beg, 0);
    y = pos(pos::type::beg, 0);
    dimensions = dims(
        dim(dim::type::abs, static_cast<float>(window.impl.GetWidth())),
        dim(dim::type::abs, static_cast<float>(window.impl.GetHeight()))
    );

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