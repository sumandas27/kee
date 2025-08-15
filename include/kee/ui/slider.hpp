#pragma once

#include "kee/ui/base.hpp"

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
        bool centered,
        std::optional<int> z_order
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

    kee::ui::mouse_state slider_state;

    unsigned int id_trans_fill_color;
    unsigned int id_trans_thumb_scale;

    unsigned int id_fill;
    unsigned int id_thumb;
};

enum class slider::event
{
    on_down,
    on_release
};

} // namespace ui
} // namespace kee