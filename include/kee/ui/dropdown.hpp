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
        const kee::ui::required& reqs,
        const kee::pos& x,
        const kee::pos& y,
        const std::variant<kee::dims, kee::border>& dimensions,
        bool centered,
        std::vector<std::string>&& options,
        std::size_t start_idx
    );

    std::function<void(std::size_t)> on_select;

private:
    void on_element_mouse_move(const raylib::Vector2& mouse_pos, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;

    void update_element(float dt) override;
    void render_element() const override;

    const std::size_t num_options;

    kee::transition<kee::color>& dropdown_outline_color;
    kee::transition<float>& dropdown_img_rotation;
    kee::transition<float>& options_height;
    kee::transition<float>& options_curr_rect_y;

    kee::ui::rect dropdown_rect;
    kee::ui::button dropdown_button;
    kee::ui::base dropdown_text_frame;
    kee::ui::text dropdown_text;
    kee::ui::base dropdown_img_frame;
    kee::ui::image dropdown_img;

    kee::ui::rect options_rect;
    kee::ui::rect options_rect_border;
    kee::ui::rect options_curr_rect;
    std::vector<kee::ui::button> options_buttons;
    std::vector<kee::ui::base> options_button_text_frames;
    std::vector<kee::ui::text> options_button_texts;

    raylib::Rectangle options_render_rect;
    bool is_dropped_down;
};

} // namespace ui
} // namespace kee