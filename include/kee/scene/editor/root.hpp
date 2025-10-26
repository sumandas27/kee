#pragma once

#include "kee/scene/editor/metadata_tab.hpp"
#include "kee/scene/editor/compose_tab.hpp"
#include "kee/scene/editor/decoration_tab.hpp"
#include "kee/scene/editor/timing_tab.hpp"

namespace kee {
namespace scene {
namespace editor {

class root final : public kee::scene::base
{
public:
    root(const kee::scene::window& window, kee::game& game, kee::global_assets& assets);

    void set_error(std::string_view error_str, bool from_file_dialog);

private:
    static constexpr float error_transition_time = 0.5f;

    enum class tabs;

    void update_element(float dt) override;

    compose_tab_info compose_info;

    kee::image_texture exit_png;
    kee::image_texture error_png;

    std::variant<
        kee::ui::handle<metadata_tab>,
        kee::ui::handle<compose_tab>,
        kee::ui::handle<decoration_tab>,
        kee::ui::handle<timing_tab>
    > active_tab_elem;
    root::tabs active_tab;

    std::vector<std::reference_wrapper<kee::transition<kee::color>>> tab_button_text_colors;
    kee::transition<float>& tab_active_rect_rel_x;
    kee::transition<float>& exit_button_rect_alpha;
    kee::transition<float>& error_rect_rel_x;
    kee::transition<float>& error_alpha;

    kee::ui::handle<kee::ui::rect> tab_rect;
    kee::ui::handle<kee::ui::base> tab_display_frame;
    kee::ui::handle<kee::ui::rect> tab_active_rect;
    std::vector<kee::ui::handle<kee::ui::button>> tab_buttons;
    std::vector<kee::ui::handle<kee::ui::text>> tab_button_text;

    kee::ui::handle<kee::ui::button> exit_button;
    kee::ui::handle<kee::ui::rect> exit_button_rect;
    kee::ui::handle<kee::ui::image> exit_button_image;

    kee::ui::handle<kee::ui::rect> error_rect;
    kee::ui::handle<kee::ui::base> error_img_frame;
    kee::ui::handle<kee::ui::image> error_img;
    kee::ui::handle<kee::ui::text> error_text;

    std::optional<int> error_skips_before_start;
    float error_timer;
};

enum class root::tabs
{
    metadata,
    compose,
    decoration,
    timing
};

} // namespace editor
} // namespace scene
} // namespace kee