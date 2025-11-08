#pragma once

#include "kee/ui/base.hpp"
#include "kee/ui/button.hpp"
#include "kee/ui/image.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/text.hpp"

namespace kee {
namespace ui {

class file_dialog_filter
{
public:
    file_dialog_filter(std::string_view name, std::string_view spec);

    std::string name;
    std::string spec;
};

class file_dialog : public kee::ui::base
{
public:
    file_dialog(
        const kee::ui::required& reqs,
        const kee::pos& x,
        const kee::pos& y,
        const std::variant<kee::dims, kee::border>& dimensions,
        bool centered,
        std::vector<file_dialog_filter> filters
    );

    std::function<void(const std::filesystem::path&)> on_success;
    std::function<void()> on_filter_mismatch;

private:
    void on_element_mouse_move(const raylib::Vector2& mouse_pos, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_element_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;

    void update_element(float dt) override;
    void render_element() const override;

    kee::transition<kee::color>& fd_outline_color;

    kee::ui::rect fd_rect;
    
    kee::ui::base fd_text_frame;
    kee::ui::text fd_text;

    kee::ui::button fd_button;
    kee::ui::image fd_image;

    std::vector<file_dialog_filter> filters;
};

} // namespace ui
} // namespace kee