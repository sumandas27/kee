#pragma once

#include "kee/ui/base.hpp"
#include "kee/ui/button.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/text.hpp"

namespace kee {
namespace ui {

class textbox;

class cursor_idx
{
public:
    cursor_idx(std::size_t char_idx, float pos_x);

    std::size_t char_idx;
    float pos_x;
};

class on_down_idxs
{
public:
    on_down_idxs(const cursor_idx& idx);

    cursor_idx beg;
    cursor_idx end;

    std::optional<float> scroll_pos;
};

class cursor
{
public:
    static constexpr float blink_time = 0.5f;

    cursor(textbox& parent, const cursor_idx& idx);

    kee::ui::rect ui;

    std::size_t char_idx;
    float blink_timer;
};

class multiselect
{
public:
    multiselect(textbox& parent, const cursor_idx& beg, const cursor_idx& end);

    kee::ui::rect ui;

    std::size_t beg_char_idx;
    std::size_t end_char_idx;
};

class textbox : public kee::ui::base
{
public:
    textbox(
        const kee::ui::required& reqs,
        const kee::pos& x,
        const kee::pos& y,
        const std::variant<kee::dims, kee::border>& dimensions,
        bool centered,
        boost::optional<kee::ui::base&> keyboard_owner,
        std::string_view initial_str
    );

    /**
     * Change external state based on a new textbox string input. Return a boolean
     * indicating if string is valid, if invalid textbox reverts to previous string.
     */
    std::function<bool(std::string_view)> on_string_input;

private:
    static constexpr float scroll_velocity = 0.5f;

    bool on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_char_press(char c) override;

    void on_element_mouse_move(const raylib::Vector2& mouse_pos, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;

    void update_element(float dt) override;
    void render_element() const override;

    float char_idx_to_pos_x(std::size_t char_idx) const;
    cursor_idx pos_x_to_char_idx(float mouse_pos_x, std::optional<bool> snap_left) const;
    
    const boost::optional<kee::ui::base&> keyboard_owner;

    kee::transition<kee::color>& textbox_outline_color;

    kee::ui::rect textbox_rect;

    kee::ui::base textbox_text_frame;
    kee::ui::text textbox_text;

    mouse_state textbox_state;
    std::optional<on_down_idxs> char_idxs;
    std::optional<float> scroll_vel;
    std::optional<std::variant<cursor, multiselect>> selection_ui;

    std::string old_str;
    bool has_render_priority;
};

} // namespace ui
} // namespace kee