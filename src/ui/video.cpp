#include "kee/ui/video.hpp"

namespace kee {
namespace ui {

video::video(
    const kee::ui::required& reqs, 
    const kee::pos& x, 
    const kee::pos& y, 
    const std::variant<kee::dims, kee::border>& dimensions, 
    bool centered
) :
    kee::ui::base(reqs, x, y, dimensions, centered)
{ }
    
} // namespace ui
} // namespace kee