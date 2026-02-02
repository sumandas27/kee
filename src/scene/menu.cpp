#include "kee/scene/menu.hpp"

#include "kee/game.hpp"
#include "kee/scene/editor/root.hpp"

namespace kee {
namespace scene {

music_analyzer::music_analyzer(const std::filesystem::path& music_path) :
    wave(music_path.string()),
    frame_cursor(0),
    fft_pcm_floats{},
    fft_prev_mags{},
    visualizer_bins{}
{ 
    SetAudioStreamBufferSizeDefault(music_analyzer::frames_per_refresh);
    audio_stream = LoadAudioStream(music_analyzer::sample_rate, music_analyzer::bit_depth, music_analyzer::channels);

    wave.Format(music_analyzer::sample_rate, music_analyzer::bit_depth, music_analyzer::channels);
    samples = std::span<sample_t>(static_cast<sample_t*>(wave.data), wave.frameCount * music_analyzer::channels);
}

music_analyzer::~music_analyzer()
{
    UnloadAudioStream(audio_stream);
}

void music_analyzer::update()
{
    while (IsAudioStreamProcessed(audio_stream))
    {
        std::array<sample_t, music_analyzer::fft_resolution * music_analyzer::channels> to_submit;
        for (std::size_t i = 0; i < music_analyzer::fft_resolution; i++)
        {
            std::size_t cursor = frame_cursor + i;
            if (cursor >= wave.frameCount)
            {
                frame_cursor = 0;
                break;
            }

            float pcm_float = 0;
            for (unsigned int ch = 0; ch < music_analyzer::channels; ch++)
            {
                const sample_t sample = samples[cursor * music_analyzer::channels + ch];
                to_submit[i * music_analyzer::channels + ch] = sample;
                pcm_float += sample;
            }

            pcm_float /= (music_analyzer::channels * std::numeric_limits<sample_t>::max());
            fft_pcm_floats[i] = pcm_float;
        }

        UpdateAudioStream(audio_stream, to_submit.data(), music_analyzer::frames_per_refresh);
        frame_cursor += music_analyzer::frames_per_refresh;
    }

    if (!IsAudioStreamPlaying(audio_stream))
        return;

    for (std::size_t i = 0; i < music_analyzer::fft_resolution; i++)
    {
        const float x = (2.f * std::numbers::pi_v<float> * i) / (music_analyzer::fft_resolution - 1.f);
        const float blackman_weight = 0.42f - 0.5f * std::cosf(x) + 0.08f * std::cosf(2 * x);
        fft_work_buffer[i] = fft_pcm_floats[i] * blackman_weight;
    }

    int j = 0;
    for (int i = 1; i < music_analyzer::fft_resolution - 1; i++)
    {
        int bit = music_analyzer::fft_resolution >> 1;
        while (j >= bit)
        {
            j -= bit;
            bit >>= 1;
        }

        j += bit;
        if (i < j)
            std::swap(fft_work_buffer[i], fft_work_buffer[j]);
    }

    for (int len = 2; len <= music_analyzer::fft_resolution; len <<= 1)
    {
        const float angle = -2.f * std::numbers::pi_v<float> / len;
        const std::complex<float> twiddle_unit(std::cosf(angle), std::sinf(angle));

        for (int i = 0; i < music_analyzer::fft_resolution; i += len)
        {
            std::complex<float> twiddle_current(1.f, 0.f);
            for (int k = 0; k < len / 2; k++)
            {
                const std::complex<float> e = fft_work_buffer[i + k];
                const std::complex<float> o = fft_work_buffer[i + k + len / 2];
                const std::complex<float> twiddle_odd(
                    o.real() * twiddle_current.real() - o.imag() * twiddle_current.imag(),
                    o.real() * twiddle_current.imag() + o.imag() * twiddle_current.real()
                );

                fft_work_buffer[i + k].real(e.real() + twiddle_odd.real());
                fft_work_buffer[i + k].imag(e.imag() + twiddle_odd.imag());

                fft_work_buffer[i + k + len / 2].real(e.real() - twiddle_odd.real());
                fft_work_buffer[i + k + len / 2].imag(e.imag() - twiddle_odd.imag());

                const float real_next = twiddle_current.real() * twiddle_unit.real() - twiddle_current.imag() * twiddle_unit.imag();
                const float imag_next = twiddle_current.real() * twiddle_unit.imag() + twiddle_current.imag() * twiddle_unit.real();
                twiddle_current.real(real_next);
                twiddle_current.imag(imag_next);
            }
        }
    }

    for (std::size_t bin = 0; bin < music_analyzer::fft_bins; bin++)
    {
        const float linear_mag = std::abs(fft_work_buffer[bin]) / music_analyzer::fft_resolution;
        const float prev_contribution = music_analyzer::smoothing_time_const * fft_prev_mags[bin];
        const float curr_contribution = (1.f - music_analyzer::smoothing_time_const) * linear_mag;
        
        const float smoothed_mag = prev_contribution + curr_contribution;
        fft_prev_mags[bin] = smoothed_mag;

        const float db = 20 * std::log10f(std::max(smoothed_mag, 1e-40f));
        const float normalized = (db - music_analyzer::min_db) * music_analyzer::inv_db_range;
        fft_curr_mags[bin] = std::clamp(normalized, 0.f, 1.f);
    }

    for (std::size_t bin = 0; bin < music_analyzer::bins; bin++)
    {
        static constexpr std::size_t fft_bin_beg = 4;
        static constexpr std::size_t fft_bin_end = 1200;
        static constexpr float spectrum_scale = 2.5f;

        const float fft_bin = std::powf(static_cast<float>(bin) / music_analyzer::bins, spectrum_scale) * (fft_bin_end - fft_bin_beg) + fft_bin_beg;
        const std::size_t fft_bin_floor = static_cast<std::size_t>(std::floor(fft_bin));
        
        const float fft_bin_decimal = fft_bin - std::floor(fft_bin);
        const float fft_bin_prev_contribution = fft_curr_mags[fft_bin_floor] * fft_bin_decimal;
        const float fft_bin_next_contribution = fft_curr_mags[fft_bin_floor + 1] * (1.f - fft_bin_decimal);
        visualizer_bins[bin] = fft_bin_prev_contribution + fft_bin_next_contribution;
    }

    std::array<float, music_analyzer::bins> avg_transform;
    for (std::size_t bin = 0; bin < 2 * music_analyzer::bins; bin++)
    {
        static constexpr std::size_t bin_dc = 0;
        static constexpr std::size_t bin_nyquist = music_analyzer::bins - 1;

        const std::size_t pass = bin / music_analyzer::bins;
        const std::size_t i = bin % music_analyzer::bins;

        float value = 0.f;
        switch (i)
        {
        case bin_dc:
            value = visualizer_bins[i];
            break;
        case bin_nyquist:
            value = visualizer_bins[i - 1] * 0.5f + visualizer_bins[i] * 0.5f;
            break;
        default: {
            const float prev_bin = visualizer_bins[i - 1];  
            const float curr_bin = visualizer_bins[i];  
            const float next_bin = visualizer_bins[i + 1];

            if (curr_bin < prev_bin || curr_bin < next_bin)
                value = (pass == 0)
                    ? curr_bin / 2.f + std::max(prev_bin, next_bin) / 2.f
                    : curr_bin / 2.f + std::max(prev_bin, next_bin) / 3.f + std::min(prev_bin, next_bin) / 6.f;
            else
                value = curr_bin;
        }}

        avg_transform[i] = std::min(value, 1.f);
        if (i == music_analyzer::bins - 1)
            visualizer_bins = avg_transform;
    }

    for (std::size_t i = 0; i < music_analyzer::bins; i++)
    {
        static constexpr float min_margin_weight = 0.7f;
        static constexpr float margin_decay = 1.6f;
        static constexpr std::size_t head_margin = 7;

        float tail_transform_factor = 1.f;
        if (i < head_margin)
        {
            const float head_margin_slope = (1.f - min_margin_weight) / std::powf(static_cast<float>(head_margin), margin_decay);
            tail_transform_factor = head_margin_slope * std::powf(static_cast<float>(i) + 1.f, margin_decay) + min_margin_weight;
        }

        visualizer_bins[i] *= tail_transform_factor;
    }

    static constexpr std::size_t smoothing_points = 3;
    static_assert(smoothing_points % 2 == 1, "smoothing_points must be odd");

    static constexpr std::size_t side_points = smoothing_points / 2;
    static constexpr int side_points_int = static_cast<int>(side_points);
    static constexpr float cn = 1.f / (2.f * side_points + 1.f);

    std::array<float, music_analyzer::bins> savitsky_golay_smooth_workspace = visualizer_bins;
    for (std::size_t i = 0; i < side_points; i++)
    {
        visualizer_bins[i] = savitsky_golay_smooth_workspace[i];
        visualizer_bins[music_analyzer::bins - 1 - i] = savitsky_golay_smooth_workspace[music_analyzer::bins - 1 - i];
    }

    for (std::size_t i = side_points; i < music_analyzer::bins - side_points; i++)
    {
        float sum = 0.f;
        for (int n = -side_points_int; n <= side_points_int; n++)
            sum += savitsky_golay_smooth_workspace[i + n] + n;
            
        visualizer_bins[i] = sum * cn;
    }

    for (std::size_t i = 0; i < music_analyzer::bins; i++)
    {
        static constexpr float spectrum_max_exp = 6.f;
        static constexpr float spectrum_min_exp = 3.f;
        static constexpr float spectrum_exponent_scale = 2.f;

        const float exp = (spectrum_max_exp - spectrum_min_exp) * (1.f - std::powf(static_cast<float>(i) / music_analyzer::bins, spectrum_exponent_scale)) + spectrum_min_exp;
        const float bin_final = std::powf(visualizer_bins[i], exp);
        visualizer_bins[i] = std::max(bin_final, 0.f);
    }
}

float music_analyzer::get_time_length() const
{
    return static_cast<float>(wave.frameCount) / static_cast<float>(sample_rate);
}

float music_analyzer::get_time_played() const
{
    return static_cast<float>(frame_cursor) / static_cast<float>(sample_rate);
}

void music_analyzer::seek(float time)
{
    time = std::clamp(time, 0.f, get_time_length());
    frame_cursor = static_cast<unsigned int>(time * sample_rate);
}

bool music_analyzer::is_playing() const
{
    return IsAudioStreamPlaying(audio_stream);
}

void music_analyzer::pause()
{
    PauseAudioStream(audio_stream);
}

void music_analyzer::play()
{
    PlayAudioStream(audio_stream);
}

void music_analyzer::resume()
{
    ResumeAudioStream(audio_stream);
}

void music_analyzer::set_volume(float new_volume)
{
    const float new_volume_clamped = std::clamp(new_volume, 0.f, 1.f);
    SetAudioStreamVolume(audio_stream, new_volume_clamped);
}

float music_analyzer::get_visualizer_bin(std::size_t i)
{
    if (i >= music_analyzer::bins)
        throw std::runtime_error("music_analyzer::get_visualizer_bin: out of bounds");

    return visualizer_bins[i];
}

opening_transitions::opening_transitions(menu& menu_scene) :
    k_rect_alpha(menu_scene.add_transition<float>(0.0f)),
    k_rect_x(menu_scene.add_transition<float>(0.5f)),
    e1_text_alpha(menu_scene.add_transition<float>(0.0f)),
    e1_rect_alpha(menu_scene.add_transition<float>(0.0f)),
    e2_text_alpha(menu_scene.add_transition<float>(0.0f)),
    e2_rect_alpha(menu_scene.add_transition<float>(0.0f)),
    e2_rect_x(menu_scene.add_transition<float>(0.5f))
{
    k_rect_alpha.set(std::nullopt, 255.0f, 0.5f, kee::transition_type::exp);
    k_rect_x.set(std::nullopt, 0.25f, 0.5f, kee::transition_type::exp);
    e1_text_alpha.set(std::nullopt, 255.0f, 0.5f, kee::transition_type::exp);
    e1_rect_alpha.set(std::nullopt, 255.0f, 0.5f, kee::transition_type::exp);
    e2_text_alpha.set(std::nullopt, 255.0f, 0.5f, kee::transition_type::exp);
    e2_rect_alpha.set(std::nullopt, 255.0f, 0.5f, kee::transition_type::exp);
    e2_rect_x.set(std::nullopt, 0.75f, 0.5f, kee::transition_type::exp);
}

music_transitions::music_transitions(menu& menu_scene) :
    menu_scene(menu_scene),
    slider_alpha(menu_scene.add_transition<float>(0.0f)),
    slider_width(menu_scene.add_transition<float>(0.0f)),
    pause_play_color(menu_scene.add_transition<kee::color>(kee::color(255, 255, 255, 0))),
    step_l_color(menu_scene.add_transition<kee::color>(kee::color(255, 255, 255, 0))),
    step_r_color(menu_scene.add_transition<kee::color>(kee::color(255, 255, 255, 0))),
    setting_color(menu_scene.add_transition<kee::color>(kee::color(255, 255, 255, 0))),
    exit_color(menu_scene.add_transition<kee::color>(kee::color(255, 255, 255, 0))),
    pause_play_border(menu_scene.add_transition<float>(0.f)),
    step_l_border(menu_scene.add_transition<float>(0.f)),
    step_r_border(menu_scene.add_transition<float>(0.f)),
    setting_border(menu_scene.add_transition<float>(0.f)),
    exit_border(menu_scene.add_transition<float>(0.f)),
    song_ui_alpha(menu_scene.add_transition<float>(0.f)),
    step_texture("assets/img/step.png"),
    setting_texture("assets/img/settings.png"),
    music_slider(menu_scene.add_child<kee::ui::slider>(2,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.895f),
        dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::rel, 0.01f)
        ),
        true, true
    )),
    pause_play(menu_scene.add_child<kee::ui::button>(2,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.95f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.04f)
        ),
        true
    )),
    pause_play_img(pause_play.ref.add_child<kee::ui::image>(std::nullopt,
        menu_scene.assets.play_png, pause_play_color.get(),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        border(border::type::rel_h, pause_play_border.get()),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    step_l(menu_scene.add_child<kee::ui::button>(2,
        pos(pos::type::rel, 0.4f),
        pos(pos::type::rel, 0.95f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.04f)
        ),
        true
    )),
    step_l_img(step_l.ref.add_child<kee::ui::image>(std::nullopt,
        step_texture, step_l_color.get(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, step_l_border.get()),
        true, ui::image::display::shrink_to_fit, true, false, 0.0f
    )),
    step_r(menu_scene.add_child<kee::ui::button>(2,
        pos(pos::type::rel, 0.6f),
        pos(pos::type::rel, 0.95f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.04f)
        ),
        true
    )),
    step_r_img(step_r.ref.add_child<kee::ui::image>(std::nullopt,
        step_texture, step_r_color.get(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, step_r_border.get()),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    setting_exit_frame(menu_scene.add_child<kee::ui::base>(2,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_w, 0.02f),
        true
    )),
    setting_button(setting_exit_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::beg, 0.f),
        pos(pos::type::beg, 0.f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.04f)
        ),
        false
    )),
    setting_img(setting_button.ref.add_child<kee::ui::image>(std::nullopt,
        setting_texture, setting_color.get(),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        border(border::type::rel_h, setting_border.get()),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    exit_button(setting_exit_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::end, 0.f),
        pos(pos::type::beg, 0.f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.04f)
        ),
        false
    )),
    exit_img(exit_button.ref.add_child<kee::ui::image>(std::nullopt,
        menu_scene.assets.exit_png, exit_color.get(),
        pos(pos::type::rel, 0.5),
        pos(pos::type::rel, 0.5),
        border(border::type::rel_h, exit_border.get()),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    ))
{
    slider_alpha.set(std::nullopt, 255.0f, 0.5f, kee::transition_type::lin);
    slider_width.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
    pause_play_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::lin);
    step_l_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::lin);
    step_r_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::lin);
    setting_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::lin);
    exit_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::lin);
    song_ui_alpha.set(std::nullopt, 255.f, 0.5f, kee::transition_type::lin);

    music_slider.ref.on_event = [&, music_is_playing = this->menu_scene.analyzer.is_playing()](ui::slider::event slider_event) mutable
    {
        switch (slider_event)
        {
        case ui::slider::event::on_down:
            music_is_playing = this->menu_scene.analyzer.is_playing();
            this->menu_scene.analyzer.pause();
            break;
        case ui::slider::event::on_release:
            this->menu_scene.analyzer.seek(this->music_slider.ref.progress * this->menu_scene.analyzer.get_time_length());
            if (music_is_playing)
                this->menu_scene.analyzer.resume();
            break;
        default:
            break;
        }
    };

    pause_play.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            pause_play_color.set(std::nullopt, kee::color(200, 200, 200), 0.5f, kee::transition_type::exp);
            pause_play_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            pause_play_border.set(std::nullopt, 0.05f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            pause_play_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
            pause_play_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    pause_play.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    { 
        if (this->menu_scene.analyzer.is_playing())
        {
            this->menu_scene.analyzer.pause();
            this->pause_play_img.ref.set_image(this->menu_scene.assets.pause_png);
        }
        else
        {
            this->menu_scene.analyzer.seek(this->music_slider.ref.progress * this->menu_scene.analyzer.get_time_length());
            this->menu_scene.analyzer.resume();
            this->pause_play_img.ref.set_image(this->menu_scene.assets.play_png);
        }
    };

    step_l.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            step_l_color.set(std::nullopt, kee::color(200, 200, 200), 0.5f, kee::transition_type::exp);
            step_l_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            step_l_border.set(std::nullopt, 0.05f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            step_l_color.set(std::nullopt, kee::color(255, 255, 255), 0.5f, kee::transition_type::exp);
            step_l_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    step_l.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    { 
        /* TODO: impl*/
    };

    step_r.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            step_r_color.set(std::nullopt, kee::color(200, 200, 200), 0.5f, kee::transition_type::exp);
            step_r_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            step_r_border.set(std::nullopt, 0.05f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            step_r_color.set(std::nullopt, kee::color(255, 255, 255), 0.5f, kee::transition_type::exp);
            step_r_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    step_r.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    { 
        /* TODO: impl*/
    };

    setting_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            setting_color.set(std::nullopt, kee::color(200, 200, 200), 0.5f, kee::transition_type::exp);
            setting_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            setting_border.set(std::nullopt, 0.05f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            setting_color.set(std::nullopt, kee::color(255, 255, 255), 0.5f, kee::transition_type::exp);
            setting_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    setting_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    { 
        /* TODO: impl*/
    };

    exit_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            exit_color.set(std::nullopt, kee::color(200, 200, 200), 0.5f, kee::transition_type::exp);
            exit_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_down_l:
            exit_border.set(std::nullopt, 0.05f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            exit_color.set(std::nullopt, kee::color(255, 255, 255), 0.5f, kee::transition_type::exp);
            exit_border.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    setting_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    { 
        /* TODO: impl*/
    };
}

menu::menu(kee::game& game, kee::global_assets& assets, const beatmap_dir_info& beatmap_info) :
    kee::scene::base(game, assets),
    k_text_alpha(add_transition<float>(0.0f)),
    k_scale(add_transition<float>(1.0f)),
    e1_scale(add_transition<float>(1.0f)),
    e2_scale(add_transition<float>(1.0f)),
    edit_text_alpha(add_transition<float>(0.f)),
    play_text_alpha(add_transition<float>(0.f)),
    browse_text_alpha(add_transition<float>(0.f)),
    k_button(add_child<kee::ui::button>(2,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.25f)
        ),
        true
    )),
    k_rect(k_button.ref.add_child<kee::ui::rect>(2,
        kee::color::blank,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::rel, 1.f)
        ),
        true,
        ui::rect_outline(ui::rect_outline::type::rel_h, 0.025f, kee::color(255, 255, 255, 0)),
        std::nullopt
    )),
    k_text(k_rect.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(255, 255, 255, k_text_alpha.get()),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.8f),
        std::nullopt, true, assets.font_semi_bold, "K", false
    )),
    edit_text(k_button.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(255, 255, 255, edit_text_alpha.get()),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, -0.08f),
        ui::text_size(ui::text_size::type::rel_h, 0.1f),
        std::nullopt, true, assets.font_regular, "EDIT", false
    )),
    e1_button(add_child<kee::ui::button>(2,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.25f)
        ),
        true
    )),
    e1_rect(e1_button.ref.add_child<kee::ui::rect>(2,
        kee::color::blank,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::rel, 1.f)
        ),
        true,
        ui::rect_outline(ui::rect_outline::type::rel_h, 0.025f, kee::color(255, 255, 255, 0)),
        std::nullopt
    )),
    e1_text(e1_rect.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(255, 255, 255, 0),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.8f),
        std::nullopt, true, assets.font_semi_bold, "E", false
    )),
    play_text(e1_button.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(255, 255, 255, play_text_alpha.get()),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, -0.08f),
        ui::text_size(ui::text_size::type::rel_h, 0.1f),
        std::nullopt, true, assets.font_regular, "PLAY", false
    )),
    e2_button(add_child<kee::ui::button>(2,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.25f)
        ),
        true
    )),
    e2_rect(e2_button.ref.add_child<kee::ui::rect>(2,
        kee::color::blank,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::rel, 1.f)
        ),
        true,
        ui::rect_outline(ui::rect_outline::type::rel_h, 0.025f, kee::color(255, 255, 255, 0)),
        std::nullopt
    )),
    e2_text(e2_rect.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(255, 255, 255, 0),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.8f),
        std::nullopt, true, assets.font_semi_bold, "E", false
    )),
    browse_text(e2_button.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(255, 255, 255, browse_text_alpha.get()),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, -0.08f),
        ui::text_size(ui::text_size::type::rel_h, 0.1f),
        std::nullopt, true, assets.font_regular, "BROWSE", false
    )),
    music_cover_art_texture(beatmap_info.dir_state.has_image
        ? std::make_optional(beatmap_info.dir_state.path / beatmap_dir_state::standard_img_filename)
        : std::nullopt
    ),
    analyzer(beatmap_info.dir_state.path / beatmap_dir_state::standard_music_filename),
    music_time(0.f),
    song_ui_frame_outer(add_child<kee::ui::base>(2,
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 0.9f),
        dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::rel, 0.1f)
        ),
        false
    )),
    song_ui_frame_inner(song_ui_frame_outer.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.25f),
        true
    )),
    music_cover_art_frame(song_ui_frame_inner.ref.add_child<kee::ui::rect>(std::nullopt,
        kee::color(255, 255, 255, 0),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::aspect, static_cast<float>(kee::window_w) / kee::window_h),
            dim(dim::type::rel, 1)
        ),
        false, std::nullopt, std::nullopt
    )),
    music_cover_art(music_cover_art_frame.ref.add_child<kee::ui::image>(std::nullopt,
        /* TODO: if cover art texture doesn't exist handle */
        music_cover_art_texture.value(), 
        kee::color(255, 255, 255, 0),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    music_info_text_frame(song_ui_frame_outer.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.25f),
        dims(
            dim(dim::type::rel, 0.01f),
            dim(dim::type::rel, 0.5f)
        ),
        false
    )),
    music_name_text(music_info_text_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(255, 255, 255, 0),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        ui::text_size(ui::text_size::type::rel_h, 0.6f),
        std::nullopt, false, assets.font_semi_bold, beatmap_info.song_name, false
    )),
    music_artist_text(music_info_text_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(255, 255, 255, 0),
        pos(pos::type::beg, 0),
        pos(pos::type::end, 0),
        ui::text_size(ui::text_size::type::rel_h, 0.35f),
        std::nullopt, false, assets.font_regular, beatmap_info.song_artist, true
    )),
    music_time_text(song_ui_frame_inner.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(255, 255, 255, 0),
        pos(pos::type::end, 0),
        pos(pos::type::beg, 0),
        ui::text_size(ui::text_size::type::rel_h, 1.0f),
        std::nullopt, false, assets.font_regular, std::string(), false
    )),
    scene_time(0.f)
{ 
    const raylib::Rectangle music_cover_art_frame_rect = music_cover_art_frame.ref.get_raw_rect();
    const float music_cover_art_frame_rel_beg = music_cover_art_frame_rect.x / kee::window_w;
    const float music_cover_art_frame_rel_end = (music_cover_art_frame_rect.x + music_cover_art_frame_rect.width)/ kee::window_w;
    music_info_text_frame.ref.x.val = music_cover_art_frame_rel_end + music_cover_art_frame_rel_beg / 2.f;

    k_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (!music_trns.has_value())
            return;

        switch (button_event)
        {
        case ui::button::event::on_hot:
            k_scale.set(std::nullopt, 1.1f, 0.5f, kee::transition_type::exp);
            edit_text_alpha.set(std::nullopt, 255.f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            k_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            edit_text_alpha.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        }
    };

    k_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (!music_trns.has_value())
            return;

        /* TODO: when exiting after transitioning to this scene from here, causes bad optional access 
            think it might not be just from here actually
        */
        game_ref.queue_scene_set<kee::scene::editor::root>(std::nullopt);
    };

    e1_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (!music_trns.has_value())
            return;

        switch (button_event)
        {
        case ui::button::event::on_hot:
            e1_scale.set(std::nullopt, 1.1f, 0.5f, kee::transition_type::exp);
            play_text_alpha.set(std::nullopt, 255.f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            e1_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            play_text_alpha.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        }
    };

    e1_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (!music_trns.has_value())
            return;

        /* TODO: impl */
    };

    e2_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (!music_trns.has_value())
            return;

        switch (button_event)
        {
        case ui::button::event::on_hot:
            e2_scale.set(std::nullopt, 1.1f, 0.5f, kee::transition_type::exp);
            browse_text_alpha.set(std::nullopt, 255.f, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            e2_scale.set(std::nullopt, 1.0f, 0.5f, kee::transition_type::exp);
            browse_text_alpha.set(std::nullopt, 0.f, 0.5f, kee::transition_type::exp);
            break;
        }
    };

    e2_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (!music_trns.has_value())
            return;

        /* TODO: impl */
    };

    visualizer_bot.reserve(music_analyzer::bins);
    visualizer_top.reserve(music_analyzer::bins);
    for (std::size_t i = 0; i < music_analyzer::bins; i++)
    {
        visualizer_bot.push_back(add_child<kee::ui::rect>(1,
            kee::color(255, 255, 255, 30),
            pos(pos::type::rel, (0.075f + static_cast<float>(i)) / music_analyzer::bins),
            pos(pos::type::end, 0),
            dims(
                dim(dim::type::rel, 0.85f / music_analyzer::bins),
                dim(dim::type::rel, 0.f)
            ),
            false, std::nullopt, std::nullopt
        ));

        visualizer_top.push_back(add_child<kee::ui::rect>(1,
            kee::color(255, 255, 255, 30),
            pos(pos::type::rel, (0.075f + static_cast<float>(i)) / music_analyzer::bins),
            pos(pos::type::beg, 0),
            dims(
                dim(dim::type::rel, 0.85f / music_analyzer::bins),
                dim(dim::type::rel, 0.f)
            ),
            false, std::nullopt, std::nullopt
        ));
    }

    k_text_alpha.set(std::nullopt, 255.0f, 2.0f, kee::transition_type::lin);
    analyzer.set_volume(1.f);
}

void menu::update_element(float dt)
{
    scene_time += dt;
    if (scene_time >= 3.0f && !opening_trns.has_value())
        opening_trns.emplace(*this);

    if (scene_time >= 3.5f && !music_trns.has_value())
    {
        music_trns.emplace(*this);
        analyzer.play();
    }

    if (music_trns.has_value())
    {
        if (!music_trns.value().music_slider.ref.is_down())
        {
            if (analyzer.is_playing())
                music_time = analyzer.get_time_played();
            
            music_trns.value().music_slider.ref.progress = music_time / analyzer.get_time_length();
        }
        else
            music_time = music_trns.value().music_slider.ref.progress * analyzer.get_time_length();

        music_trns.value().music_slider.ref.color.a = music_trns.value().slider_alpha.get();
        std::get<kee::dims>(music_trns.value().music_slider.ref.dimensions).w.val = music_trns.value().slider_width.get();

        std::get<kee::border>(music_trns.value().pause_play_img.ref.dimensions).val = music_trns.value().pause_play_border.get();
        std::get<kee::border>(music_trns.value().step_l_img.ref.dimensions).val = music_trns.value().step_l_border.get();
        std::get<kee::border>(music_trns.value().step_r_img.ref.dimensions).val = music_trns.value().step_r_border.get();
        std::get<kee::border>(music_trns.value().setting_img.ref.dimensions).val = music_trns.value().setting_border.get();
        std::get<kee::border>(music_trns.value().exit_img.ref.dimensions).val = music_trns.value().exit_border.get();

        music_trns.value().pause_play_img.ref.color = music_trns.value().pause_play_color.get();
        music_trns.value().step_l_img.ref.color = music_trns.value().step_l_color.get();
        music_trns.value().step_r_img.ref.color = music_trns.value().step_r_color.get();
        music_trns.value().setting_img.ref.color = music_trns.value().setting_color.get();
        music_trns.value().exit_img.ref.color = music_trns.value().exit_color.get();

        const unsigned int music_length = static_cast<unsigned int>(analyzer.get_time_length());
        const unsigned int music_time_int = static_cast<unsigned int>(music_time);
        const std::string music_time_str = std::format("{}:{:02} / {}:{:02}", music_time_int / 60, music_time_int % 60, music_length / 60, music_length % 60);
        music_time_text.ref.set_string(music_time_str);

        music_cover_art_frame.ref.color.a = (20.f * music_trns.value().song_ui_alpha.get()) / 255.f;
        music_time_text.ref.color.a = music_trns.value().song_ui_alpha.get();
        music_name_text.ref.color.a = music_trns.value().song_ui_alpha.get();
        music_artist_text.ref.color.a = (100.f * music_trns.value().song_ui_alpha.get()) / 255.f;
        music_cover_art.ref.color.a = music_trns.value().song_ui_alpha.get();
    }

    k_text.ref.color.a = k_text_alpha.get();
    k_text.ref.set_scale(k_scale.get());
    edit_text.ref.set_scale(k_scale.get());
    edit_text.ref.color.a = edit_text_alpha.get();
    k_rect.ref.outline.value().color.a = opening_trns.has_value() ? opening_trns.value().k_rect_alpha.get() : 0.f;
    k_button.ref.x.val = opening_trns.has_value() ? opening_trns.value().k_rect_x.get() : 0.5f;
    std::get<kee::dims>(k_button.ref.dimensions).h.val = 0.25f * k_scale.get();

    e1_text.ref.color.a = opening_trns.has_value() ? opening_trns.value().e1_text_alpha.get() : 0.f;
    e1_text.ref.set_scale(e1_scale.get());
    play_text.ref.set_scale(e1_scale.get());
    play_text.ref.color.a = play_text_alpha.get();
    e1_rect.ref.outline.value().color.a = opening_trns.has_value() ? opening_trns.value().e1_rect_alpha.get() : 0.f;
    std::get<kee::dims>(e1_button.ref.dimensions).h.val = 0.25f * e1_scale.get();

    e2_text.ref.color.a = opening_trns.has_value() ? opening_trns.value().e2_text_alpha.get() : 0.f;
    e2_text.ref.set_scale(e2_scale.get());
    browse_text.ref.set_scale(e2_scale.get());
    browse_text.ref.color.a = browse_text_alpha.get();
    e2_rect.ref.outline.value().color.a = opening_trns.has_value() ? opening_trns.value().e2_rect_alpha.get() : 0.f;
    e2_button.ref.x.val = opening_trns.has_value() ? opening_trns.value().e2_rect_x.get() : 0.5f;
    std::get<kee::dims>(e2_button.ref.dimensions).h.val = 0.25f * e2_scale.get();

    analyzer.update();
    for (std::size_t i = 0; i < music_analyzer::bins; i++)
    {
        std::get<kee::dims>(visualizer_bot[i].ref.dimensions).h.val = 0.5f * analyzer.get_visualizer_bin(i);
        std::get<kee::dims>(visualizer_top[i].ref.dimensions).h.val = 0.5f * analyzer.get_visualizer_bin(music_analyzer::bins - i - 1);
    }
}

} // namespace scene
} // namespace kee