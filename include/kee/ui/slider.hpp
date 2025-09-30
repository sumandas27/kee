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
        const kee::ui::base::required& reqs,
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
    void handle_element_events() override;
    void update_element(float dt) override;

    kee::transition<kee::color>& fill_color;
    kee::transition<float>& thumb_scale;

    /* TODO: make these not children */
    kee::ui::handle<kee::ui::rect> track;
    kee::ui::handle<kee::ui::rect> fill;
    kee::ui::handle<kee::ui::rect> thumb;

    kee::mouse_state slider_state;
};

enum class slider::event
{
    on_down,
    on_release
};

} // namespace ui
} // namespace kee