#pragma once

#include "kee/ui/base.hpp"
#include "kee/ui/button.hpp"
#include "kee/ui/image.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/text.hpp"

namespace kee {
namespace ui {

class dropdown final : public kee::ui::base
{
public:
    dropdown(
        const kee::ui::base::required& reqs,
        kee::pos x,
        kee::pos y,
        const std::variant<kee::dims, kee::border>& dimensions,
        bool centered,
        std::vector<std::string>&& options,
        std::size_t start_idx
    );

private:
    void on_element_mouse_move(const raylib::Vector2& mouse_pos, bool ctrl_modifier) override;
    bool on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, bool ctrl_modifier) override;
    bool on_element_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, bool ctrl_modifier) override;

    void update_element(float dt) override;
    void render_element() const override;

    kee::transition<kee::color>& dropdown_outline_color;
    kee::transition<float>& dropdown_img_rotation;
    kee::transition<float>& options_height;

    kee::ui::rect dropdown_rect;
    kee::ui::base dropdown_text_frame;
    kee::ui::text dropdown_text;
    kee::ui::base dropdown_button_frame;
    kee::ui::button dropdown_button;
    kee::ui::image dropdown_img;

    kee::ui::rect options_rect;

    bool is_dropped_down;

    std::vector<std::string> options;
};

} // namespace ui
} // namespace kee