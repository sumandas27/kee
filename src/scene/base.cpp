#include "kee/scene/base.hpp"

namespace kee {
namespace scene {

required::required(kee::game& game_ref, kee::global_assets& assets) :
    game_ref(game_ref),
    assets(assets)
{ }

/* TODO: some infinite loops going on here investigate */

void base::on_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods)
{
    if (element_keyboard_capture.has_value())
        element_keyboard_capture.value().on_key_down(keycode, mods);
    //else
    //    kee::ui::base::on_key_down(keycode, mods);
}

void base::on_key_up(int keycode, magic_enum::containers::bitset<kee::mods> mods)
{
    if (element_keyboard_capture.has_value())
        element_keyboard_capture.value().on_key_up(keycode, mods);
    //else
    //    kee::ui::base::on_key_up(keycode, mods);
}

void base::on_char_press(char c)
{
    if (element_keyboard_capture.has_value())
        element_keyboard_capture.value().on_char_press(c);
    //else
    //    kee::ui::base::on_char_press(c);
}

bool base::on_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods)
{
    if (render_priority.has_value() && render_priority.value().on_mouse_down(mouse_pos, is_mouse_l, mods))
        return true;
    else
        return kee::ui::base::on_mouse_down(mouse_pos, is_mouse_l, mods);
}

bool base::on_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods)
{
    if (render_priority.has_value() && render_priority.value().on_mouse_up(mouse_pos, is_mouse_l, mods))
        return true;
    else
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