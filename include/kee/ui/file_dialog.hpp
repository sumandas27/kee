#pragma once

#include "kee/ui/base.hpp"
#include "kee/ui/button.hpp"
#include "kee/ui/image.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/text.hpp"

namespace kee {
namespace ui {

class file_dialog : public kee::ui::base
{
public:
    file_dialog(
        const kee::ui::required& reqs,
        const kee::pos& x,
        const kee::pos& y,
        const std::variant<kee::dims, kee::border>& dimensions,
        bool centered,
        std::vector<std::string_view> filters,
        std::variant<std::string_view, std::filesystem::path> initial_msg
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
    kee::ui::button fd_button;
    kee::ui::image fd_image;

    kee::ui::base fd_text_area;
    kee::ui::base fd_text_frame;
    kee::ui::text fd_text;

    std::vector<std::string_view> filters;
    std::optional<std::filesystem::path> old_path;
};

} // namespace ui
} // namespace kee