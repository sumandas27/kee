#pragma once

#include "kee/ui/base.hpp"
#include "kee/ui/rect.hpp"

namespace kee {
namespace ui {

class slider final : public kee::ui::base
{
public:
    enum class event;

    slider(
        const kee::ui::required& reqs,
        kee::pos x, 
        kee::pos y, 
        const std::variant<kee::dims, kee::border>& dimensions,
        bool centered
    );

    bool is_down() const;

    std::function<void(slider::event)> on_event;

    /**
     * Scaled from 0.0f to 1.0f.
     */
    float progress;

private:
    void on_element_mouse_move(const raylib::Vector2& mouse_pos, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;    

    void update_element(float dt) override;
    void render_element() const override;

    kee::transition<kee::color>& fill_color;
    kee::transition<float>& thumb_scale;

    kee::ui::rect track;
    kee::ui::rect fill;
    kee::ui::rect thumb;

    kee::mouse_state slider_state;
};

enum class slider::event
{
    on_down,
    on_release
};

} // namespace ui
} // namespace kee