#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {

class button : public kee::ui::base
{
public:
    enum class event;

    button(
        const kee::ui::base::required& reqs, 
        kee::pos x, 
        kee::pos y, 
        const std::variant<kee::dims, kee::border>& dimensions, 
        bool centered
    );

    std::function<void(button::event, magic_enum::containers::bitset<kee::mods>)> on_event;
    std::function<void(magic_enum::containers::bitset<kee::mods>)> on_click_l;
    std::function<void(magic_enum::containers::bitset<kee::mods>)> on_click_r;

    void on_element_mouse_move(const raylib::Vector2& mouse_pos, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;

private:
    kee::mouse_state button_state;

    std::optional<bool> is_down_l;
};

enum class button::event
{
    on_hot,
    on_down_l,
    on_down_r,
    on_leave
};

} // namespace ui
} // namespace kee