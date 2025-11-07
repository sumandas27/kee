#pragma once

#include "kee/ui/base.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/text.hpp"
#include "kee/ui/textbox.hpp"

namespace kee {
namespace scene {
namespace editor {

class root;

class timing_tab_info
{
public:
    timing_tab_info(const raylib::Music& music, const std::optional<float>& prev_beat);

    const raylib::Music& music;
    const std::optional<float>& prev_beat;

    raylib::Sound metronome_hi;
    raylib::Sound metronome_lo;
};

class timing_tab : public kee::ui::base
{
public:
    timing_tab(const kee::ui::required& reqs, root& root_elem, timing_tab_info& timing_info);

private:
    static const kee::color timing_rect_color;

    void update_element(float dt) override;

    root& root_elem;
    timing_tab_info& timing_info;

    kee::ui::handle<kee::ui::base> points_frame;
    kee::ui::handle<kee::ui::rect> points_bg;
    kee::ui::handle<kee::ui::text> wip_text;

    std::vector<kee::ui::handle<kee::ui::rect>> timing_rects;
    kee::ui::handle<kee::ui::rect> timing_slider;
    kee::ui::handle<kee::ui::base> timing_ball_frame;
    std::vector<kee::ui::handle<kee::ui::rect>> timing_slider_ticks;
    kee::ui::handle<kee::ui::rect> timing_ball;

    kee::ui::handle<kee::ui::base> bpm_offset_frame;
    kee::ui::handle<kee::ui::text> bpm_text;
    kee::ui::handle<kee::ui::textbox> bpm_textbox;

    kee::ui::handle<kee::ui::text> offset_text;
    kee::ui::handle<kee::ui::textbox> offset_textbox;
};

} // namespace editor
} // namespace scene
} // namespace kee