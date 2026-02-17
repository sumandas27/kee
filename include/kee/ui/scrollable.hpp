#pragma once

#include "kee/ui/base.hpp"

/* TODO: mouse input */

namespace kee {
namespace ui {

class scrollable : public kee::ui::base
{
public:
    scrollable(
        const kee::ui::required& reqs,
        const kee::pos& x,
        const kee::pos& y,
        const std::variant<kee::dims, kee::border>& dimensions,
        bool centered
    );

private:
    void render() const override;
};

} // namespace ui
} // namespace kee