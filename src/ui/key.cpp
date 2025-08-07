#include "kee/ui/key.hpp"

#include "kee/ui/rect.hpp"
#include "kee/ui/text.hpp"

namespace kee {
namespace ui {

key::key(kee::ui::base& parent, int key_id, const raylib::Vector2& relative_pos) :
    kee::ui::base(parent,
        pos(pos::type::rel, relative_pos.x),
        pos(pos::type::rel, relative_pos.y),
        dims(
            dim(dim::type::aspect, key_id == KeyboardKey::KEY_SPACE ? 7.0f : 1.0f),
            dim(dim::type::rel, 0.25)
        ),
        true, std::nullopt, false
    ),
    is_pressed(false),
    keycode(key_id)
{
    /* TODO: parent dimension verification ??? */

    set_color(raylib::Color::White());

    id_rect = add_child_no_id<kee::ui::rect>(
        std::nullopt,
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        border(border::type::rel_h, 0.05f),
        border(border::type::rel_h, 0.02f), 
        true, std::nullopt, false
    );

    const std::string key_str = (key_id != KeyboardKey::KEY_SPACE) 
        ? std::string(1, static_cast<char>(key_id)) 
        : "___";

    child_at(id_rect)->add_child_no_id<kee::ui::text>(
        std::nullopt,
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        text_size(text_size::type::rel_h, 0.5),
        key_str, true, std::nullopt, false
    );
}

void key::update_element([[maybe_unused]] float dt)
{
    set_color(raylib::Keyboard::IsKeyDown(keycode) ? raylib::Color::Green() : raylib::Color::White());
}

key::ui_data::ui_data(int raylib_key, const raylib::Vector2& relative_pos) :
    raylib_key(raylib_key),
    relative_pos(relative_pos)
{ }

} // namespace ui
} // namespace kee