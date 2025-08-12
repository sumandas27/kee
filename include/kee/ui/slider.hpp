#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {

class slider final : public kee::ui::base
{
public:
    slider(
        const kee::ui::base& parent,
        kee::pos x, 
        kee::pos y, 
        const std::variant<kee::dims, kee::border>& dimensions,
        bool centered,
        std::optional<int> z_order
    );

    float progress;

private:
    void update_element(float dt) override;

    unsigned int id_fill;
};

} // namespace ui
} // namespace kee