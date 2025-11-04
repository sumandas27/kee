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
            dim(dim::type::rel, 0.88f)
        ),
        false
    ),
    points_frame(add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 0.5f),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    points_bg(points_frame.ref.add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(20, 20, 20),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_w, 0.05f),
        true, std::nullopt,
        ui::rect_roundness(ui::rect_roundness::type::rel_w, 0.05f, std::nullopt)
    )),
    timing_slider(add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(40, 40, 40),
        pos(pos::type::rel, 0.55f),
        pos(pos::type::rel, 0.4f),
        dims(
            dim(dim::type::rel, 0.4f),
            dim(dim::type::rel, 0.05f)
        ),
        false, std::nullopt,
        ui::rect_roundness(ui::rect_roundness::type::rel_h, 0.5f, std::nullopt)
    )),
    bpm_offset_frame(add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.725f),
        pos(pos::type::rel, 0.75f),
        dims(
            dim(dim::type::rel, 0.15f),
            dim(dim::type::rel, 0.1f)
        ),
        true
    )),
    bpm_text(bpm_offset_frame.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        ui::text_size(ui::text_size::type::rel_h, 0.4f),
        false, assets.font_semi_bold, "BPM", false
    )),
    bpm_textbox(bpm_offset_frame.ref.add_child<kee::ui::textbox>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 0.5f),
            dim(dim::type::rel, 0.4f)
        ),
        false, *this
    )),
    offset_text(bpm_offset_frame.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.6f),
        ui::text_size(ui::text_size::type::rel_h, 0.4f),
        false, assets.font_semi_bold, "Offset", false
    )),
    offset_textbox(bpm_offset_frame.ref.add_child<kee::ui::textbox>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.6f),
        dims(
            dim(dim::type::rel, 0.5f),
            dim(dim::type::rel, 0.4f)
        ),
        false, *this
    ))
{ 
    timing_rects.reserve(4);
    for (std::size_t i = 0; i < 4; i++)
        timing_rects.emplace_back(add_child<kee::ui::rect>(std::nullopt,
            raylib::Color(40, 40, 40),
            pos(pos::type::rel, 0.55f + 0.095f * i),
            pos(pos::type::rel, 0.2f),
            dims(
                dim(dim::type::rel, 0.065f),
                dim(dim::type::aspect, 1)
            ),
            false, std::nullopt,
            ui::rect_roundness(ui::rect_roundness::type::rel_w, 0.05f, std::nullopt)
        ));
}

} // namespace editor
} // namespace scene
} // namespace kee