#include "kee/ui/scissor.hpp"

#include "kee/game.hpp"

namespace kee {
namespace ui {

scissor::scissor(
    const kee::ui::required& reqs,
    const kee::pos& x,
    const kee::pos& y,
    const std::variant<kee::dims, kee::border>& dimensions,
    bool centered
) :
    kee::ui::base(reqs, x, y, dimensions, centered)
{ }

void scissor::render() const
{
    game_ref.scissor_mode.push(get_raw_rect());
    kee::ui::base::render();
    game_ref.scissor_mode.pop();
}

} // namespace ui
} // namespace kee