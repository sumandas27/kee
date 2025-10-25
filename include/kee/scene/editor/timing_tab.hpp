#pragma once

#include "kee/ui/base.hpp"
#include "kee/ui/text.hpp"

namespace kee {
namespace scene {
namespace editor {

class timing_tab : public kee::ui::base
{
public:
    timing_tab(const kee::ui::required& reqs);

private:
    kee::ui::handle<kee::ui::text> wip_text;
};

} // namespace editor
} // namespace scene
} // namespace kee