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
};

class cursor
{
public:
    cursor(textbox& parent, const cursor_idx& idx);

    kee::ui::rect ui;

    float blink_timer;
};

class multiselect
{
public:
    multiselect(textbox& parent, const cursor_idx& beg, const cursor_idx& end);

    kee::ui::rect ui;
};

class textbox : public kee::ui::base
{
public:
    textbox(
        const kee::ui::required& reqs,
        kee::pos x,
        kee::pos y,
        const std::variant<kee::dims, kee::border>& dimensions,
        bool centered
    );

private:
    void on_element_mouse_move(const raylib::Vector2& mouse_pos, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;

    void update_element(float dt) override;
    void render_element() const override;

    cursor_idx get_curr_cursor_idx(const raylib::Vector2& mouse_pos) const;

    kee::transition<kee::color>& textbox_outline_color;

    kee::ui::rect textbox_rect;

    kee::ui::base textbox_text_frame;
    kee::ui::text textbox_text;

    mouse_state textbox_state;
    std::optional<on_down_idxs> char_idxs;
    std::optional<std::variant<cursor, multiselect>> selection_ui;

    bool has_render_priority;
};

} // namespace ui
} // namespace kee