#include "kee/scene/editor/timing_tab.hpp"

#include "kee/scene/editor/root.hpp"

namespace kee {
namespace scene {
namespace editor {

timing_tab_info::timing_tab_info() :
    metronome_hi("assets/sfx/metronome_hi.wav"),
    metronome_lo("assets/sfx/metronome_lo.wav")
{ 
    metronome_hi.SetVolume(0.2f);
    metronome_lo.SetVolume(0.2f);
}

timing_tab::timing_tab(const kee::ui::required& reqs, root& root_elem, song_ui& song_ui_elem, timing_tab_info& timing_info) :
    kee::ui::base(reqs,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.04f),
        dims(
            dim(dim::type::rel, 1.0f),
            dim(dim::type::rel, 0.88f)
        ),
        false
    ),
    root_elem(root_elem),
    song_ui_elem(song_ui_elem),
    timing_info(timing_info),
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
    wip_text(points_bg.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.05f),
        true, assets.font_semi_bold, "WORK IN PROGRESS", false
    )),
    timing_slider(add_child<kee::ui::rect>(std::nullopt,
        timing_tab::timing_rect_color.to_color(),
        pos(pos::type::rel, 0.55f),
        pos(pos::type::rel, 0.4f),
        dims(
            dim(dim::type::rel, 0.4f),
            dim(dim::type::rel, 0.03f)
        ),
        false, std::nullopt,
        ui::rect_roundness(ui::rect_roundness::type::rel_h, 0.5f, std::nullopt)
    )),
    timing_ball_frame(timing_slider.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::abs, 0),
            dim(dim::type::rel, 1)
        ),
        true
    )),
    timing_ball(timing_ball_frame.ref.add_child<kee::ui::rect>(1,
        raylib::Color::White(),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.75f)
        ),
        true, std::nullopt,
        ui::rect_roundness(ui::rect_roundness::type::rel_h, 0.5f, std::nullopt)
    )),
    bpm_offset_frame(add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.725f),
        pos(pos::type::rel, 0.7f),
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
        false, *this, std::format("{:.2f}", song_ui_elem.music_bpm)
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
        false, *this, std::format("{:.2f}", song_ui_elem.music_start_offset)
    ))
{ 
    const raylib::Rectangle timing_slider_rect = timing_slider.ref.get_raw_rect();
    std::get<kee::dims>(timing_ball_frame.ref.dimensions).w.val = timing_slider_rect.width - timing_slider_rect.height;

    for (std::size_t i = 0; i <= 4; i++)
        timing_slider_ticks.emplace_back(timing_ball_frame.ref.add_child<kee::ui::rect>(0,
            raylib::Color::DarkGray(),
            pos(pos::type::rel, i / 4.0f),
            pos(pos::type::rel, 0.5f),
            dims(
                dim(dim::type::aspect, 1),
                dim(dim::type::rel, 0.4f)
            ),
            true, std::nullopt,
            ui::rect_roundness(ui::rect_roundness::type::rel_h, 0.5f, std::nullopt)
        ));

    timing_rects.reserve(4);
    for (std::size_t i = 0; i < 4; i++)
        timing_rects.emplace_back(add_child<kee::ui::rect>(std::nullopt,
            timing_tab::timing_rect_color.to_color(),
            pos(pos::type::rel, 0.55f + 0.11f * i),
            pos(pos::type::rel, 0.2f),
            dims(
                dim(dim::type::rel, 0.065f),
                dim(dim::type::aspect, 1)
            ),
            false, std::nullopt,
            ui::rect_roundness(ui::rect_roundness::type::rel_w, 0.05f, std::nullopt)
        ));

    bpm_textbox.ref.on_string_input = [&](std::string_view new_str) -> bool
    {
        float text_float;

        auto [ptr, ec] = std::from_chars(new_str.data(), new_str.data() + new_str.size(), text_float);
        if (ec != std::errc() || ptr != new_str.data() + new_str.size())
            return false;

        if (text_float < 1 || text_float > 1000)
        {
            this->root_elem.set_error("BPM range: 1 to 1000", false);
            return false;
        }

        if (this->song_ui_elem.music_bpm != text_float)
        {
            this->song_ui_elem.music_bpm = text_float;
            if (this->root_elem.save_info.has_value())
                this->root_elem.save_info.value().save_metadata_needed = true;
        }
        
        return true;
    };

    offset_textbox.ref.on_string_input = [&](std::string_view new_str) -> bool
    {
        float text_float;

        auto [ptr, ec] = std::from_chars(new_str.data(), new_str.data() + new_str.size(), text_float);
        if (ec != std::errc() || ptr != new_str.data() + new_str.size())
            return false;

        if (text_float < -10)
        {
            this->root_elem.set_error("Offset must be greater than -10 seconds.", false);
            return false;
        }

        if (this->song_ui_elem.music_start_offset != text_float)
        {
            this->song_ui_elem.music_start_offset = text_float;
            if (this->root_elem.save_info.has_value())
                this->root_elem.save_info.value().save_metadata_needed = true;
        }
        
        return true;
    };
}

const kee::color timing_tab::timing_rect_color = kee::color(40, 40, 40);

void timing_tab::update_element([[maybe_unused]] float dt)
{
    if (song_ui_elem.get_music().IsPlaying() && song_ui_elem.get_prev_beat().has_value())
    {
        const int first = static_cast<int>(std::floor(song_ui_elem.get_prev_beat().value())) + 1;
        const int last = static_cast<int>(std::floor(song_ui_elem.get_beat()));

        for (int i = first; i <= last; i++)
            if (i % 4 == 0)
                timing_info.metronome_hi.Play();
            else
                timing_info.metronome_lo.Play();
    }

    const float beat = song_ui_elem.get_beat();
    if (beat >= 0.0f)
    {
        const std::size_t color_idx = static_cast<std::size_t>(beat) % 4;
        const float interpolation = beat - std::floor(beat);

        for (std::size_t i = 0; i < timing_rects.size(); i++)
        {
            raylib::Color rect_color;
            if (i == color_idx)
            {
                const kee::color kee_color = timing_tab::timing_rect_color * interpolation + kee::color::green * (1.0f - interpolation);
                rect_color = kee_color.to_color();
            }
            else
                rect_color = timing_tab::timing_rect_color.to_color();

            timing_rects[i].ref.set_opt_color(rect_color);
        }

        timing_ball.ref.x.val = (color_idx % 2 == 0) ? interpolation : 1.0f - interpolation;
    }
    else
    {
        for (kee::ui::handle<kee::ui::rect>& timing_rect : timing_rects)
            timing_rect.ref.set_opt_color(timing_tab::timing_rect_color.to_color());

        timing_ball.ref.x.val = 0;
    }
}

} // namespace editor
} // namespace scene
} // namespace kee