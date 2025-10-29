#include "kee/ui/textbox.hpp"

namespace kee {
namespace ui {

textbox::textbox(
    const kee::ui::required& reqs,
    kee::pos x,
    kee::pos y,
    const std::variant<kee::dims, kee::border>& dimensions,
    bool centered
) :
    kee::ui::base(reqs, x, y, dimensions, centered),
    textbox_outline_color(add_transition<kee::color>(kee::color::white())),
    textbox_rect(make_temp_child<kee::ui::rect>(
        raylib::Color(50, 50, 50),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        false,
        rect_outline(rect_outline::type::rel_h, 0.07f, textbox_outline_color.get().to_color()),
        std::nullopt
    )),
    textbox_text_frame(make_temp_child<kee::ui::base>(
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.1f),
        true
    )),
    textbox_text(textbox_text_frame.make_temp_child<kee::ui::text>(
        raylib::Color::White(),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        text_size(text_size::type::rel_h, 1),
        false, assets.font_regular, std::string(), false
    ))
{ }

void textbox::on_element_mouse_move(const raylib::Vector2& mouse_pos, magic_enum::containers::bitset<kee::mods> mods)
{
    const raylib::Rectangle raw_rect = get_raw_rect();
    const bool is_mouse_on_textbox =
        mouse_pos.x >= raw_rect.x && mouse_pos.x <= raw_rect.x + raw_rect.width &&
        mouse_pos.y >= raw_rect.y && mouse_pos.y <= raw_rect.y + raw_rect.height;

    if (!is_mouse_on_textbox && textbox_state != mouse_state::off)
    {
        textbox_outline_color.set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
        textbox_state = mouse_state::off;
    }
    else if (is_mouse_on_textbox && textbox_state == mouse_state::off)
    {
        textbox_outline_color.set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
        textbox_state = mouse_state::hot;
    }
}

bool textbox::on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods)
{
    if (textbox_state != mouse_state::down || !is_mouse_l)
        return false;

    return false;
}

bool textbox::on_element_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods)
{
    /* TODO: complete */
    return false;
}

void textbox::update_element([[maybe_unused]] float dt)
{
    textbox_rect.border.value().opt_color = textbox_outline_color.get().to_color();
}

void textbox::render_element() const
{
    textbox_rect.render();
    textbox_text.render();
}

} // namespace ui
} // namespace kee