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
    char_idx(idx.char_idx),
    blink_timer(cursor::blink_time)
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
    )),
    beg_char_idx(beg.char_idx),
    end_char_idx(end.char_idx)
{ }

textbox::textbox(
    const kee::ui::required& reqs,
    kee::pos x,
    kee::pos y,
    const std::variant<kee::dims, kee::border>& dimensions,
    bool centered,
    boost::optional<kee::ui::base&> keyboard_owner
) :
    kee::ui::base(reqs, x, y, dimensions, centered),
    keyboard_owner(keyboard_owner),
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
        false, assets.font_regular, "TEST STRING raylib::Keyboard::IsKeyPressedRepeat(key) raylib::Keyboard::IsKeyPressedRepeat(key)", false
    )),
    textbox_state(mouse_state::off),
    has_render_priority(false)
{ }

bool textbox::on_element_key_down(int keycode, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    switch (keycode)
    {
    case KeyboardKey::KEY_BACKSPACE: {
        if (!selection_ui.has_value())
            return false;
        
        std::optional<cursor_idx> cursor_ctor_param;
        std::visit([&](auto&& ui) 
        {
            using T = std::decay_t<decltype(ui)>;
            if constexpr (std::is_same_v<T, cursor>)
            {
                if (ui.char_idx != 0)
                {
                    ui.char_idx--;
                    std::string new_string = this->textbox_text.get_string();
                    new_string.erase(ui.char_idx, 1);
                    this->textbox_text.set_string(new_string);

                    ui.ui.x.val = this->char_idx_to_pos_x(ui.char_idx);
                }
            }
            else if constexpr (std::is_same_v<T, multiselect>)
            {
                std::string new_string = this->textbox_text.get_string();
                new_string.erase(ui.beg_char_idx, ui.end_char_idx - ui.beg_char_idx);
                this->textbox_text.set_string(new_string);

                const std::size_t cursor_char_idx = ui.beg_char_idx;
                cursor_ctor_param = cursor_idx(cursor_char_idx, this->char_idx_to_pos_x(cursor_char_idx));
            }
        }, 
        selection_ui.value());

        if (cursor_ctor_param.has_value())
            selection_ui.emplace(cursor(*this, cursor_ctor_param.value()));

        cursor& cursor_ui = std::get<cursor>(selection_ui.value());
        const float new_char_pos_x = cursor_ui.ui.get_raw_rect().x;
        const std::size_t curr_idx = cursor_ui.char_idx;

        const float textbox_frame_x = textbox_text_frame.get_raw_rect().x;
        if (new_char_pos_x < textbox_frame_x)
        {
            float textbox_text_x_change = textbox_frame_x - new_char_pos_x;
            if (curr_idx > 0)
            {
                const GlyphInfo gi = GetGlyphInfo(textbox_text.font, static_cast<int>(textbox_text.get_string()[curr_idx - 1]));
                const float char_size = gi.advanceX * textbox_text.get_base_scale();
                textbox_text_x_change += char_size * 0.8f;
            }

            textbox_text.x.val += textbox_text_x_change;
            cursor_ui.ui.x.val = char_idx_to_pos_x(cursor_ui.char_idx);
        }

        return true;
    }
    case KeyboardKey::KEY_RIGHT: {
        std::optional<cursor_idx> cursor_ctor_param;
        std::visit([&](auto&& ui) 
        {
            using T = std::decay_t<decltype(ui)>;
            if constexpr (std::is_same_v<T, cursor>)
            {
                if (ui.char_idx != this->textbox_text.get_string().size())
                {
                    ui.char_idx++;
                    ui.ui.x.val = this->char_idx_to_pos_x(ui.char_idx);

                    ui.ui.set_opt_color(kee::color::dark_orange().to_color());
                    ui.blink_timer = cursor::blink_time;
                }
            }
            else if constexpr (std::is_same_v<T, multiselect>)
                cursor_ctor_param = cursor_idx(ui.end_char_idx, this->char_idx_to_pos_x(ui.end_char_idx));
        }, 
        selection_ui.value());

        if (cursor_ctor_param.has_value())
            selection_ui.emplace(cursor(*this, cursor_ctor_param.value()));

        cursor& cursor_ui = std::get<cursor>(selection_ui.value());
        const raylib::Rectangle cursor_rect = cursor_ui.ui.get_raw_rect();
        const raylib::Rectangle text_rect = textbox_text_frame.get_raw_rect();
        
        if (cursor_rect.x + cursor_rect.width > text_rect.x + text_rect.width)
        {
            textbox_text.x.val -= (cursor_rect.x + cursor_rect.width) - (text_rect.x + text_rect.width);
            cursor_ui.ui.x.val = char_idx_to_pos_x(cursor_ui.char_idx);
        }

        return true;
    }
    case KeyboardKey::KEY_LEFT: {
        std::optional<cursor_idx> cursor_ctor_param;
        std::visit([&](auto&& ui) 
        {
            using T = std::decay_t<decltype(ui)>;
            if constexpr (std::is_same_v<T, cursor>)
            {
                if (ui.char_idx != 0)
                {
                    ui.char_idx--;
                    ui.ui.x.val = this->char_idx_to_pos_x(ui.char_idx);

                    ui.ui.set_opt_color(kee::color::dark_orange().to_color());
                    ui.blink_timer = cursor::blink_time;
                }
            }
            else if constexpr (std::is_same_v<T, multiselect>)
                cursor_ctor_param = cursor_idx(ui.beg_char_idx, this->char_idx_to_pos_x(ui.beg_char_idx));
        }, 
        selection_ui.value());

        if (cursor_ctor_param.has_value())
            selection_ui.emplace(cursor(*this, cursor_ctor_param.value()));

        cursor& cursor_ui = std::get<cursor>(selection_ui.value());
        const raylib::Rectangle cursor_rect = cursor_ui.ui.get_raw_rect();
        const float textbox_frame_x = textbox_text_frame.get_raw_rect().x;

        if (cursor_rect.x < textbox_frame_x)
        {
            textbox_text.x.val += textbox_frame_x - cursor_rect.x;
            cursor_ui.ui.x.val = char_idx_to_pos_x(cursor_ui.char_idx);
        }

        return true;
    }
    default:
        return false;
    }
}

void textbox::on_element_mouse_move(const raylib::Vector2& mouse_pos, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
{
    const raylib::Rectangle raw_rect = get_raw_rect();
    const bool is_mouse_on_textbox =
        mouse_pos.x >= raw_rect.x && mouse_pos.x <= raw_rect.x + raw_rect.width &&
        mouse_pos.y >= raw_rect.y && mouse_pos.y <= raw_rect.y + raw_rect.height;

    scroll_vel.reset();
    if (char_idxs.has_value())
    {
        const raylib::Rectangle textbox_frame_rect = textbox_text_frame.get_raw_rect();
        if (mouse_pos.x < textbox_frame_rect.x)
        {
            if (!char_idxs.value().scroll_pos.has_value())
            {
                char_idxs.value().end = pos_x_to_char_idx(textbox_frame_rect.x, true);
                char_idxs.value().scroll_pos = 0.0f;
            }

            scroll_vel = -(textbox_frame_rect.x - mouse_pos.x) * textbox::scroll_velocity;
        }
        else if (mouse_pos.x > textbox_frame_rect.x + textbox_frame_rect.width)
        {
            if (!char_idxs.value().scroll_pos.has_value())
            {
                char_idxs.value().end = pos_x_to_char_idx(textbox_frame_rect.x + textbox_frame_rect.width, false);
                char_idxs.value().scroll_pos = 0.0f;
            }
            
            scroll_vel = (mouse_pos.x - textbox_frame_rect.x - textbox_frame_rect.width) * textbox::scroll_velocity;
        }
        else
            char_idxs.value().end = pos_x_to_char_idx(mouse_pos.x, std::nullopt);

        if (char_idxs.value().beg.char_idx != char_idxs.value().end.char_idx)
        {
            if (std::holds_alternative<multiselect>(selection_ui.value()))
            {
                const cursor_idx& beg = (char_idxs.value().beg.pos_x < char_idxs.value().end.pos_x) ? char_idxs.value().beg : char_idxs.value().end;
                const cursor_idx& end = (char_idxs.value().beg.pos_x > char_idxs.value().end.pos_x) ? char_idxs.value().beg : char_idxs.value().end;

                multiselect& multi = std::get<multiselect>(selection_ui.value());
                std::get<kee::dims>(multi.ui.dimensions).w.val = end.pos_x - beg.pos_x;
                multi.ui.x.val = beg.pos_x;
                multi.beg_char_idx = beg.char_idx;
                multi.end_char_idx = end.char_idx;
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

            release_keyboard_capture();
            if (keyboard_owner.has_value())
                keyboard_owner.value().take_keyboard_capture();
        }
        
        return false;
    }
    else
    {
        if (!has_render_priority)
        {
            take_render_priority();
            has_render_priority = true;

            if (keyboard_owner.has_value())
                keyboard_owner.value().release_keyboard_capture();

            take_keyboard_capture();
        }

        char_idxs = on_down_idxs(pos_x_to_char_idx(mouse_pos.x, std::nullopt));
        selection_ui.emplace(cursor(*this, char_idxs.value().beg));

        textbox_state = mouse_state::down;
        return true;
    }
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

            blink_timer = cursor::blink_time;
        }
    }

    if (char_idxs.has_value())
    {
        if (scroll_vel.has_value())
        {
            char_idxs.value().scroll_pos.value() += scroll_vel.value() * dt;

            const raylib::Rectangle textbox_frame_rect = textbox_text_frame.get_raw_rect();
            if (char_idxs.value().scroll_pos < -1.0f)
            {
                const std::size_t shift_chars_l = static_cast<std::size_t>(std::floor(-char_idxs.value().scroll_pos.value()));
                const std::size_t new_char_idx = (char_idxs.value().end.char_idx >= shift_chars_l) ? char_idxs.value().end.char_idx - shift_chars_l : 0;
                
                const float new_char_pos_x = char_idx_to_pos_x(new_char_idx);
                if (new_char_pos_x < textbox_frame_rect.x)
                    textbox_text.x.val -= new_char_pos_x + (get_raw_rect().x - textbox_frame_rect.x);

                char_idxs.value().beg.pos_x = char_idx_to_pos_x(char_idxs.value().beg.char_idx);
                char_idxs.value().end = cursor_idx(new_char_idx, char_idx_to_pos_x(new_char_idx));
                char_idxs.value().scroll_pos.value() += shift_chars_l;
            }
            else if (char_idxs.value().scroll_pos > 1.0f)
            {
                const std::size_t shift_chars_r = static_cast<std::size_t>(std::floor(char_idxs.value().scroll_pos.value()));
                const std::size_t new_char_idx = std::min(char_idxs.value().end.char_idx + shift_chars_r, textbox_text.get_string().size());
                
                const float new_char_pos_x = char_idx_to_pos_x(new_char_idx);
                if (new_char_pos_x - textbox_text.x.val > textbox_frame_rect.width)
                    textbox_text.x.val += textbox_frame_rect.width - new_char_pos_x;

                char_idxs.value().beg.pos_x = char_idx_to_pos_x(char_idxs.value().beg.char_idx);
                char_idxs.value().end = cursor_idx(new_char_idx, char_idx_to_pos_x(new_char_idx));
                char_idxs.value().scroll_pos.value() -= shift_chars_r;
            }
        }
        else
            char_idxs.value().scroll_pos.reset();
    }
}

void textbox::render_element() const
{
    textbox_rect.render();

    /**
     * NOTE: `Begin/EndScissorMode` isn't available on `raylib-cpp`, so we must use the C API directly here.
     */
    const raylib::Rectangle text_frame_rect = textbox_text_frame.get_raw_rect();
    BeginScissorMode(
        static_cast<int>(text_frame_rect.x),
        static_cast<int>(text_frame_rect.y),
        static_cast<int>(text_frame_rect.width),
        static_cast<int>(text_frame_rect.height)
    );
    textbox_text.render();

    if (selection_ui.has_value())
        std::visit([](const auto& ui) {
            ui.ui.render();
        }, selection_ui.value());
    EndScissorMode();
}

float textbox::char_idx_to_pos_x(std::size_t char_idx) const
{
    float res = textbox_text.get_raw_rect().x - get_raw_rect().x;

    const std::string& textbox_string = textbox_text.get_string();
    const std::size_t max = std::min(char_idx, textbox_string.size());
    for (std::size_t i = 0; i < max; i++)
    {
        const GlyphInfo gi = GetGlyphInfo(textbox_text.font, static_cast<int>(textbox_string[i]));
        res += gi.advanceX * textbox_text.get_base_scale();
    }

    return res;
}

cursor_idx textbox::pos_x_to_char_idx(float mouse_pos_x, std::optional<bool> snap_left) const
{
    const float local_x = mouse_pos_x - textbox_text.get_raw_rect().x;
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

        float compare_x;
        if (snap_left.has_value())
        {
            if (snap_left.value())
                compare_x = curr_x;
            else
                compare_x = curr_x + char_width;
        }
        else
            compare_x = curr_x + char_width / 2.0f;

        if (local_x < compare_x)
            break;

        curr_x += char_width;
        i++;
    }

    const float text_offset = textbox_text.get_raw_rect().x - get_raw_rect().x;
    return cursor_idx(i, text_offset + curr_x);
}

} // namespace ui
} // namespace kee