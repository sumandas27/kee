#pragma once

#include "kee/ui/base.hpp"
#include "kee/ui/rect.hpp"

/* TODO: mouse input */

namespace kee {
namespace ui {

class scrollbar : public kee::ui::base
{
public:
    scrollbar(
        const kee::ui::required& reqs,
        const kee::pos& x,
        const kee::dim& w,
        float rel_h
    );

private:
    void on_element_mouse_move(const raylib::Vector2& mouse_pos, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;

    void update_element(float dt) override;

    kee::transition<kee::color>& thumb_color;

    kee::ui::handle<kee::ui::base> thumb_frame;
    kee::ui::handle<kee::ui::rect> thumb;

    std::optional<float> rel_h_down_offset;
    bool thumb_is_hot;
};

class scrollable : public kee::ui::base
{
public:
    scrollable(
        const kee::ui::required& reqs,
        const kee::pos& x,
        const kee::pos& y,
        const std::variant<kee::dims, kee::border>& dimensions,
        bool centered,
        const kee::pos& scrollbar_x,
        const kee::dim& scrollbar_w,
        const kee::pos& scroll_frame_x,
        const kee::dim& scroll_frame_w
    );

    template <std::derived_from<kee::ui::base> T, typename... Args>
    kee::ui::handle<T> add_scrollable_child(std::optional<int> z_order, Args&&... args);

    void set_scrollable_rel_h(float rel_h);

private:
    void render() const override;

    const kee::pos scrollbar_x;
    const kee::dim scrollbar_w;

    kee::ui::handle<kee::ui::rect> scroll_frame_bg;
    kee::ui::handle<kee::ui::base> scroll_frame;

    std::optional<kee::ui::handle<scrollbar>> scrollbar_ui;
};

} // namespace ui
} // namespace kee

#include "kee/ui/scrollable.ipp"