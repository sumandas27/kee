#pragma once

#include "kee/ui/base.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/text.hpp"
#include "kee/ui/textbox.hpp"

namespace kee {
namespace scene {
namespace editor {

class timing_tab : public kee::ui::base
{
public:
    timing_tab(const kee::ui::required& reqs);

private:
    kee::ui::handle<kee::ui::base> points_frame;
    kee::ui::handle<kee::ui::rect> points_bg;

    std::vector<kee::ui::handle<kee::ui::rect>> timing_rects;
    kee::ui::handle<kee::ui::rect> timing_slider;

    kee::ui::handle<kee::ui::base> bpm_offset_frame;
    kee::ui::handle<kee::ui::text> bpm_text;
    kee::ui::handle<kee::ui::textbox> bpm_textbox;

    kee::ui::handle<kee::ui::text> offset_text;
    kee::ui::handle<kee::ui::textbox> offset_textbox;
};

} // namespace editor
} // namespace scene
} // namespace kee