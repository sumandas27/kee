#include "kee/scene/editor/timing_tab.hpp"

namespace kee {
namespace scene {
namespace editor {

timing_tab::timing_tab(const kee::ui::required& reqs) :
    kee::ui::base(reqs,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.04f),
        dims(
            dim(dim::type::rel, 1.0f),
            dim(dim::type::rel, 0.96f)
        ),
        false
    ),
    wip_text(add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.07f),
        true, assets.font_semi_bold, "WORK IN PROGRESS", false
    ))
{ }

} // namespace editor
} // namespace scene
} // namespace kee