#include "kee/ui/textbox.hpp"

namespace kee {
namespace ui {

cursor_idx::cursor_idx(std::size_t char_idx, float pos_x) :
    char_idx(char_idx),
    pos_x(pos_x)
{ }

on_down_idxs::on_down_idxs(const cursor_idx& idx) :
    beg(idx),
    end(idx)
{ }

cursor::cursor(textbox& parent, const cursor_idx& idx) :
    ui(parent.make_temp_child<kee::ui::rect>(
        raylib::Color(255, 140, 0, 255),
        pos(pos::type::beg, idx.pos_x),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 0.07f),
            dim(dim::type::rel, 0.65f)
        ),
        true, std::nullopt, std::nullopt
    )),
    blink_timer(0.5f)
{ }

multiselect::multiselect(textbox& parent, const cursor_idx& beg, const cursor_idx& end) :
    ui(parent.make_temp_child<kee::ui::rect>(
        raylib::Color(255, 255, 255, 128),
        pos(pos::type::beg, beg.pos_x),
        pos(pos::type::rel, 0.175f),
        dims(
            dim(dim::type::abs, end.pos_x - beg.pos_x),
            dim(dim::type::rel, 0.65f)
        ),
        false, std::nullopt, std::nullopt
    ))
{ }

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
        false, assets.font_regular, "TEST STRING", false
    )),
    textbox_state(mouse_state::off),
    has_render_priority(false)
{ }

void textbox::on_element_mouse_move(const raylib::Vector2& mouse_pos, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    const raylib::Rectangle raw_rect = get_raw_rect();
    const bool is_mouse_on_textbox =
        mouse_pos.x >= raw_rect.x && mouse_pos.x <= raw_rect.x + raw_rect.width &&
        mouse_pos.y >= raw_rect.y && mouse_pos.y <= raw_rect.y + raw_rect.height;

    if (char_idxs.has_value())
    {
        char_idxs.value().end = get_curr_cursor_idx(mouse_pos);
        
        if (char_idxs.value().beg.char_idx != char_idxs.value().end.char_idx)
        {
            if (std::holds_alternative<multiselect>(selection_ui.value()))
            {
                const float beg_abs_x = (char_idxs.value().beg.pos_x < char_idxs.value().end.pos_x) ? char_idxs.value().beg.pos_x : char_idxs.value().end.pos_x;
                const float end_abs_x = (char_idxs.value().beg.pos_x > char_idxs.value().end.pos_x) ? char_idxs.value().beg.pos_x : char_idxs.value().end.pos_x;

                kee::ui::rect& multiselect_ui = std::get<multiselect>(selection_ui.value()).ui;
                std::get<kee::dims>(multiselect_ui.dimensions).w.val = end_abs_x - beg_abs_x;
                multiselect_ui.x.val = beg_abs_x;
            }
            else
                selection_ui.emplace(multiselect(*this, char_idxs.value().beg, char_idxs.value().end));
        }
        else if (char_idxs.value().beg.char_idx == char_idxs.value().end.char_idx && !std::holds_alternative<cursor>(selection_ui.value()))
            selection_ui.emplace(cursor(*this, char_idxs.value().beg));
    }
    else if (is_mouse_on_textbox && textbox_state == mouse_state::off)
    {
        textbox_outline_color.set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
        textbox_state = mouse_state::hot;
    }
    else if (!is_mouse_on_textbox && textbox_state != mouse_state::off)
    {
        textbox_state = mouse_state::off;
        if (!has_render_priority)
            textbox_outline_color.set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
    }
}

bool textbox::on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    if (!is_mouse_l)
        return false;

    if (textbox_state != mouse_state::hot)
    {
        if (has_render_priority)
        {
            textbox_outline_color.set(kee::color::white());
            selection_ui.reset();
            release_render_priority();
            has_render_priority = false;
        }
        
        return false;
    }

    char_idxs = get_curr_cursor_idx(mouse_pos);
    selection_ui.emplace(cursor(*this, char_idxs.value().beg));

    take_render_priority();
    has_render_priority = true;
    textbox_state = mouse_state::down;
    return true;
}

bool textbox::on_element_mouse_up([[maybe_unused]] const raylib::Vector2& mouse_pos, bool is_mouse_l, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    if (textbox_state != mouse_state::down || !is_mouse_l)
        return false;

    char_idxs.reset();
    textbox_state = mouse_state::hot;
    return true;
}

void textbox::update_element([[maybe_unused]] float dt)
{
    textbox_rect.border.value().opt_color = textbox_outline_color.get().to_color();

    if (selection_ui.has_value() && std::holds_alternative<cursor>(selection_ui.value()) && textbox_state != mouse_state::down)
    {
        float& blink_timer = std::get<cursor>(selection_ui.value()).blink_timer;
        blink_timer -= dt;

        if (blink_timer <= 0.0f)
        {
            const unsigned char cur_alpha = std::get<cursor>(selection_ui.value()).ui.get_opt_color().value().a;
            const unsigned char new_alpha = (cur_alpha == 0) ? 255 : 0;
            std::get<cursor>(selection_ui.value()).ui.set_opt_color(raylib::Color(255, 140, 0, new_alpha));

            blink_timer = 0.5f;
        }
    }
}

void textbox::render_element() const
{
    textbox_rect.render();
    textbox_text.render();

    if (selection_ui.has_value())
        std::visit([](const auto& ui) {
            ui.ui.render();
        }, selection_ui.value());
}

cursor_idx textbox::get_curr_cursor_idx(const raylib::Vector2& mouse_pos) const
{
    const float local_x = mouse_pos.x - textbox_text.get_raw_rect().x;
    const std::string& textbox_string = textbox_text.get_string();

    float curr_x = 0.0f;
    std::size_t i = 0;
    while (i < textbox_string.size())
    {
        /**
         * Note: No `raylib-cpp` binding for `GetGlyphInfo`.
         */
        const GlyphInfo gi = GetGlyphInfo(textbox_text.font, static_cast<int>(textbox_string[i]));
        const float char_width = gi.advanceX * textbox_text.get_base_scale();
        if (local_x < curr_x + char_width / 2.0f)
            break;

        curr_x += char_width;
        i++;
    }

    const float text_offset = textbox_text.get_raw_rect().x - get_raw_rect().x;
    return cursor_idx(i, text_offset + curr_x);
}

} // namespace ui
} // namespace kee