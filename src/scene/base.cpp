#include "kee/scene/base.hpp"

namespace kee {
namespace scene {

required::required(kee::game& game_ref, kee::global_assets& assets) :
    game_ref(game_ref),
    assets(assets)
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

base::base(const kee::scene::required& reqs) :
    kee::ui::base(reqs.game_ref, reqs.assets)
{ }

} // namespace scene
} // namespace kee