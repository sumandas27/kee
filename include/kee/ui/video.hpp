#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {

class video final : public kee::ui::base 
{
public:
    video(
        const kee::ui::required& reqs, 
        const kee::pos& x, 
        const kee::pos& y, 
        const std::variant<kee::dims, kee::border>& dimensions, 
        bool centered
    );
};

} // namespace ui
} // namespace kee