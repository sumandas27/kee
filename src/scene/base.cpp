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

    /* TODO: custom window size and fix ui so fun :D */
    impl.Init(2560, 1440);
    impl.SetTargetFPS(window_fps);
}

base::base(kee::game& game, kee::global_assets& assets) :
    kee::ui::base(game, assets)
{ }

bool base::on_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods)
{
    if (render_priority.has_value() && render_priority.value().on_mouse_down(mouse_pos, is_mouse_l, mods))
        return true;

    return kee::ui::base::on_mouse_down(mouse_pos, is_mouse_l, mods);
}

bool base::on_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods)
{
    if (render_priority.has_value() && render_priority.value().on_mouse_up(mouse_pos, is_mouse_l, mods))
        return true;

    return kee::ui::base::on_mouse_up(mouse_pos, is_mouse_l, mods);
}

void base::render() const
{
    kee::ui::base::render();

    if (render_priority.has_value())
        render_priority.value().render();
}

} // namespace scene
} // namespace kee