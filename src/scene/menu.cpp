#include "kee/scene/menu.hpp"

#include <chrono>
#include <random>

#include "kee/game.hpp"
#include "kee/scene/beatmap.hpp"
#include "kee/scene/editor/root.hpp"

namespace kee {
namespace scene {

music_analyzer::music_analyzer(menu& menu_scene, std::optional<std::size_t> beatmap_id) :
    menu_scene(menu_scene),
    start_frame_idx(0),
    fft_pcm_floats{},
    fft_prev_mags{},
    visualizer_bins{},
    is_audio_stream_stopped(true)
{
    /**
     * This is the earliest `assets.play_assets` is used, all other usages of this are now valid.
     */
    if (menu_scene.assets.play_assets_future.valid())
    {
        menu_scene.assets.play_assets_future.wait();
        menu_scene.assets.play_assets = menu_scene.assets.play_assets_future.get();
    }

    for (const auto& [id, _] : menu_scene.assets.play_assets)
        beatmap_id_order.push_back(id);

    std::mt19937 rng(std::random_device{}());
    std::ranges::shuffle(beatmap_id_order, rng);

    if (beatmap_id.has_value())
    {
        auto first_it = std::ranges::find(beatmap_id_order, beatmap_id.value());
        if (first_it == beatmap_id_order.end())
            throw std::runtime_error("cannot find analyzer first beatmap");

        std::iter_swap(first_it, beatmap_id_order.begin());
    }

    SetAudioStreamBufferSizeDefault(music_analyzer::frames_per_refresh);
    audio_stream = LoadAudioStream(music_analyzer::sample_rate, music_analyzer::bit_depth, music_analyzer::channels);

    set_order_song(0);
}

music_analyzer::~music_analyzer()
{
    UnloadAudioStream(audio_stream);
}

std::size_t music_analyzer::get_beatmap_id() const
{
    const std::size_t order_idx_mod = order_idx % beatmap_id_order.size();
    return beatmap_id_order[order_idx_mod];
}

void music_analyzer::update()
{
    if (!IsAudioStreamPlaying(audio_stream))
        return;

    while (IsAudioStreamProcessed(audio_stream))
    {
        static constexpr std::size_t samples_per_refresh = music_analyzer::frames_per_refresh * music_analyzer::channels;

        std::array<sample_t, samples_per_refresh> to_submit;
        for (std::size_t i = 0; i < music_analyzer::fft_resolution; i++)
        {
            const std::size_t samples_frames = samples.size() / music_analyzer::channels;
            if (i >= samples_frames && !decode_next_packet().has_value())
                return;

            float pcm_float = 0;
            for (unsigned int ch = 0; ch < music_analyzer::channels; ch++)
            {
                const std::size_t sample_idx = i * music_analyzer::channels + ch;
                const sample_t sample = samples[sample_idx];
                pcm_float += sample;

                if (i < music_analyzer::frames_per_refresh)
                    to_submit[sample_idx] = sample;
            }

            pcm_float /= (music_analyzer::channels * std::numeric_limits<sample_t>::max());
            fft_pcm_floats[i] = pcm_float;
        }

        samples.erase(samples.begin(), samples.begin() + samples_per_refresh);
        start_frame_idx += music_analyzer::frames_per_refresh;

        UpdateAudioStream(audio_stream, to_submit.data(), music_analyzer::frames_per_refresh);
    }

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
    return static_cast<float>(audio_input.duration().seconds());
}

float music_analyzer::get_time_played() const
{
    return static_cast<float>(start_frame_idx) / static_cast<float>(sample_rate);
}

void music_analyzer::seek(float time)
{
    const av::Stream av_audio_stream = audio_input.stream(audio_stream_idx);
    const int64_t target_ts = static_cast<int64_t>(time / av_audio_stream.timeBase()());
    audio_input.seek(target_ts, static_cast<int>(audio_stream_idx), AVSEEK_FLAG_BACKWARD);

    samples.clear();
    start_frame_idx = decode_next_packet().value_or(0);

    if (!is_audio_stream_stopped && !is_playing())
    {
        StopAudioStream(audio_stream);
        is_audio_stream_stopped = true;
    }
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
    if (is_audio_stream_stopped)
    {
        PlayAudioStream(audio_stream);
        is_audio_stream_stopped = false;
    }
    else
        ResumeAudioStream(audio_stream);
}

bool music_analyzer::step_l()
{
    if (order_idx > 0)
    {
        set_order_song(order_idx - 1);
        return true;
    }
    else
    {
        seek(0.f);
        return false;
    }
}

void music_analyzer::step_r()
{
    set_order_song(order_idx + 1);
    menu_scene.set_menu_level();
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

void music_analyzer::set_order_song(std::size_t order_idx_param)
{
    order_idx = order_idx_param;

    const std::string song_path_mp3 = (menu_scene.assets.play_assets.at(get_beatmap_id()).beatmap_dir_path / beatmap_dir_state::standard_music_filename).string();
    audio_input.close();
    audio_input.openInput(song_path_mp3);
    audio_input.findStreamInfo();

    std::optional<std::size_t> opt_audio_stream_idx = std::nullopt;
    av::Stream av_audio_stream;
    for (std::size_t i = 0; i < audio_input.streamsCount(); i++) 
    {
        const av::Stream stream = audio_input.stream(i);
        if (stream.mediaType() == AVMEDIA_TYPE_AUDIO) 
        {
            opt_audio_stream_idx = i;
            av_audio_stream = stream;
            break;
        }
    }

    if (!opt_audio_stream_idx.has_value()) 
        throw std::runtime_error("Can't find audio stream");

    audio_stream_idx = opt_audio_stream_idx.value();
    if (av_audio_stream.isValid())
    {
        audio_decoder = av::AudioDecoderContext(av_audio_stream);

        av::Codec codec = av::findDecodingCodec(audio_decoder.raw()->codec_id);
        audio_decoder.setCodec(codec);
        audio_decoder.setRefCountedFrames(true);
        audio_decoder.open(av::Codec());
    }

    std::uint64_t dst_channels_layout;
    if constexpr (music_analyzer::channels == 2)
        dst_channels_layout = AV_CH_LAYOUT_STEREO;
    else
        throw std::runtime_error(std::format("Unsupported analyzer channel count {}", music_analyzer::channels));

    enum AVSampleFormat out_sample_fmt;
    if constexpr (std::is_same_v<sample_t, std::int16_t>)
        out_sample_fmt = AV_SAMPLE_FMT_S16;
    else
        throw std::runtime_error("Unsupported analyzer sample type.");

    audio_resampler.init(
        dst_channels_layout, music_analyzer::sample_rate, out_sample_fmt,
        audio_decoder.channelLayout(), audio_decoder.sampleRate(), audio_decoder.sampleFormat()
    );
    
    seek(0.f);
}

std::optional<std::size_t> music_analyzer::decode_next_packet()
{
    av::AudioSamples audio_samples;
    std::optional<double> packet_pts = std::nullopt;

    while (const av::Packet packet = audio_input.readPacket()) 
    {
        if (static_cast<std::size_t>(packet.streamIndex()) != audio_stream_idx)
            continue;

        audio_samples = audio_decoder.decode(packet);
        if (!audio_samples)
            continue;
        
        packet_pts = packet.pts().seconds();
        break;
    }

    const bool is_eof = !packet_pts.has_value();
    if (is_eof)
    { 
        step_r();
        return std::nullopt;
    }

    audio_resampler.push(audio_samples);
    audio_samples = audio_resampler.pop(0);

    const std::span<sample_t> samples_view(reinterpret_cast<sample_t*>(audio_samples.data()), audio_samples.samplesCount() * music_analyzer::channels);
    samples.insert(samples.end(), samples_view.begin(), samples_view.end());

    return static_cast<std::size_t>(packet_pts.value() * music_analyzer::sample_rate);
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
    const float trns_time = menu_scene.from_game_init ? 0.5f : 0.f;

    k_rect_alpha.set(std::nullopt, 255.0f, trns_time, kee::transition_type::exp);
    k_rect_x.set(std::nullopt, 0.25f, trns_time, kee::transition_type::exp);
    e1_text_alpha.set(std::nullopt, 255.0f, trns_time, kee::transition_type::exp);
    e1_rect_alpha.set(std::nullopt, 255.0f, trns_time, kee::transition_type::exp);
    e2_text_alpha.set(std::nullopt, 255.0f, trns_time, kee::transition_type::exp);
    e2_rect_alpha.set(std::nullopt, 255.0f, trns_time, kee::transition_type::exp);
    e2_rect_x.set(std::nullopt, 0.75f, trns_time, kee::transition_type::exp);
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
        menu_scene.assets.pause_png, pause_play_color.get(),
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
    const float trns_time = menu_scene.from_game_init ? 0.5f : 0.f;

    slider_alpha.set(std::nullopt, 255.0f, trns_time, kee::transition_type::lin);
    slider_width.set(std::nullopt, 1.0f, trns_time, kee::transition_type::exp);
    pause_play_color.set(std::nullopt, kee::color::white, trns_time, kee::transition_type::lin);
    step_l_color.set(std::nullopt, kee::color::white, trns_time, kee::transition_type::lin);
    step_r_color.set(std::nullopt, kee::color::white, trns_time, kee::transition_type::lin);
    setting_color.set(std::nullopt, kee::color::white, trns_time, kee::transition_type::lin);
    exit_color.set(std::nullopt, kee::color::white, trns_time, kee::transition_type::lin);
    song_ui_alpha.set(std::nullopt, 255.f, trns_time, kee::transition_type::lin);

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
                this->menu_scene.analyzer.play();
            break;
        default:
            break;
        }
    };

    pause_play.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (menu_scene.play_ui.has_value())
            return;

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
            this->pause_play_img.ref.set_image(this->menu_scene.assets.play_png);
        
            this->is_slider_touched_during_pause = false;
        }
        else
        {
            this->menu_scene.analyzer.play();
            this->pause_play_img.ref.set_image(this->menu_scene.assets.pause_png);
        }
    };

    step_l.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (menu_scene.play_ui.has_value())
            return;

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
        if (menu_scene.analyzer.get_time_played() < 3.f && menu_scene.analyzer.step_l())
            menu_scene.set_menu_level();
        else
            menu_scene.analyzer.seek(0.f);

        if (!menu_scene.analyzer.is_playing())
            pause_play.ref.on_click_l(magic_enum::containers::bitset<kee::mods>());
    };

    step_r.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (menu_scene.play_ui.has_value())
            return;

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
        menu_scene.analyzer.step_r();
        if (!menu_scene.analyzer.is_playing())
            pause_play.ref.on_click_l(magic_enum::containers::bitset<kee::mods>());
    };

    setting_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (menu_scene.play_ui.has_value())
            return;

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
        if (menu_scene.play_ui.has_value())
            return;

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

    exit_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        this->menu_scene.game_ref.queue_game_exit();
    };
}

level_ui::level_ui(
    const kee::ui::required& reqs,
    const kee::pos& x,
    const kee::pos& y,
    const std::variant<kee::dims, kee::border>& dims,
    bool centered,
    bool is_selected,
    play& play_ui,
    const level_ui_assets& ui_assets,
    std::size_t beatmap_id
) :
    kee::ui::button(reqs, x, y, dims, centered),
    beatmap_id(beatmap_id),
    play_ui(play_ui),
    frame_color(add_transition<kee::color>(is_selected ? kee::color(30, 30, 30) : kee::color(10, 10, 10))),
    image_frame_color(add_transition<kee::color>(is_selected ? kee::color(45, 45, 45) : kee::color(15, 15, 15))),
    frame(add_child<kee::ui::rect>(std::nullopt,
        frame_color.get(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0),
        true, std::nullopt,
        ui::rect_roundness(ui::rect_roundness::type::rel_h, 0.2f, std::nullopt)
    )),
    image_frame(frame.ref.add_child<kee::ui::rect>(std::nullopt,
        image_frame_color.get(),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        kee::dims(
            dim(dim::type::aspect, static_cast<float>(kee::window_w) / kee::window_h),
            dim(dim::type::rel, 1)
        ),
        false, std::nullopt, std::nullopt
    )),
    image(ui_assets.img.has_value() ?
        std::make_optional(image_frame.ref.add_child<kee::ui::image>(std::nullopt,
            ui_assets.img.value(), 
            kee::color::white,
            pos(pos::type::rel, 0.5f),
            pos(pos::type::rel, 0.5f),
            border(border::type::abs, 0),
            true, ui::image::display::shrink_to_fit, false, false, 0.0f
        )) :
        std::nullopt
    ),
    text_frame(frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::beg, 0.f),
        pos(pos::type::rel, 0.f),
        kee::dims(
            dim(dim::type::abs, 0.f),
            dim(dim::type::rel, 1.f)
        ),
        false
    )),
    text_inner_frame(text_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.1f),
        true
    )),
    song_name_text(text_inner_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 0.f),
        ui::text_size(ui::text_size::type::rel_h, 0.6f),
        std::nullopt, false, assets.font_semi_bold, ui_assets.song_name, false
    )),
    song_artist_text(text_inner_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(200, 200, 200),
        pos(pos::type::rel, 0.f),
        pos(pos::type::end, 0.f),
        ui::text_size(ui::text_size::type::rel_h, 0.35f),
        std::nullopt, false, assets.font_regular, ui_assets.song_artist, false
    )),
    level_name_text(text_inner_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(150, 150, 150),
        pos(pos::type::beg, 0.f),
        pos(pos::type::end, 0.f),
        ui::text_size(ui::text_size::type::rel_h, 0.35f),
        std::nullopt, false, assets.font_italic, " - " + ui_assets.mapper + "'s " + ui_assets.level_name, false
    )),
    performance_outer_frame(frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::beg, 0.f),
        pos(pos::type::rel, 0.f),
        kee::dims(
            dim(dim::type::abs, 0.f),
            dim(dim::type::rel, 1.f)
        ),
        false
    )),
    performance_frame(performance_outer_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(border::type::rel_h, 0.15f),
        true
    )),
    rating_rect(performance_frame.ref.add_child<kee::ui::rect>(std::nullopt,
        kee::color(120, 120, 120),
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 0.f),
        kee::dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::aspect, 0.32f)
        ),
        false, std::nullopt,
        ui::rect_roundness(ui::rect_roundness::type::rel_h, 0.5f, std::nullopt)
    )),
    rating_frame(rating_rect.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.45f),
        pos(pos::type::rel, 0.5f),
        kee::dims(
            dim(dim::type::abs, 0.f),
            dim(dim::type::rel, 1.f)
        ),
        true
    )),
    rating_star_img(rating_frame.ref.add_child<kee::ui::image>(std::nullopt,
        assets.star_png,
        kee::color::white,
        pos(pos::type::beg, 0.f),
        pos(pos::type::rel, 0.5f),
        kee::dims(
            dim(dim::type::aspect, 1.f),
            dim(dim::type::rel, 0.8f)
        ),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    rating_text(rating_star_img.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 1.25f),
        pos(pos::type::rel, 0.f),
        ui::text_size(ui::text_size::type::rel_h, 1.f),
        std::nullopt, false, assets.font_semi_bold, "1", false
    )),
    progress_text_frame(performance_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.f),
        pos(pos::type::end, 0.f),
        kee::dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::abs, 0.f)
        ),
        false
    )),
    progress_text(progress_text_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.9f),
        std::nullopt, true, assets.font_semi_bold,
        ui_assets.best.has_value() ? std::to_string(ui_assets.best.value().high_score) : "0", 
        true
    )),
    is_selected(is_selected)
{
    on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (this->is_selected)
            return;

        switch (button_event)
        {
        case ui::button::event::on_hot:
            frame_color.set(std::nullopt, kee::color(20, 20, 20), 0.5f, kee::transition_type::exp);
            image_frame_color.set(std::nullopt, kee::color(30, 30, 30), 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            frame_color.set(std::nullopt, kee::color(10, 10, 10), 0.5f, kee::transition_type::exp);
            image_frame_color.set(std::nullopt, kee::color(15, 15, 15), 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        this->play_ui.set_selected_level(this->beatmap_id);
    };

    const float image_frame_w = image_frame.ref.get_raw_rect().width;
    const float info_w = get_raw_rect().width - image_frame_w;
    const float text_frame_w = info_w * 0.88f;

    performance_outer_frame.ref.x.val = image_frame_w + text_frame_w;
    std::get<kee::dims>(performance_outer_frame.ref.dimensions).w.val = info_w - text_frame_w;

    text_frame.ref.x.val = image_frame_w;
    std::get<kee::dims>(text_frame.ref.dimensions).w.val = text_frame_w;
    level_name_text.ref.x.val = song_artist_text.ref.get_raw_rect().width;

    rating_text.ref.refresh_ui();
    rating_star_img.ref.x.val = rating_star_img.ref.get_raw_rect().width / 2;
    std::get<kee::dims>(rating_frame.ref.dimensions).w.val = rating_star_img.ref.get_raw_rect().width + rating_text.ref.get_raw_rect().width;
    std::get<kee::dims>(progress_text_frame.ref.dimensions).h.val = performance_frame.ref.get_raw_rect().height - rating_rect.ref.get_raw_rect().height;
}

void level_ui::select()
{
    is_selected = true;

    frame_color.set(kee::color(30, 30, 30));
    image_frame_color.set(kee::color(45, 45, 45));
}

void level_ui::unselect()
{
    is_selected = false;

    frame_color.set(std::nullopt, kee::color(15, 15, 15), 0.3f, kee::transition_type::exp);
    image_frame_color.set(std::nullopt, kee::color(10, 10, 10), 0.3f, kee::transition_type::exp);
}

void level_ui::update_element([[maybe_unused]] float dt)
{
    frame.ref.color = frame_color.get();
    image_frame.ref.color = image_frame_color.get();
}

play::play(const kee::ui::required& reqs, menu& menu_scene, std::size_t analyzer_beatmap_id) :
    kee::ui::button(reqs,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0),
        true
    ),
    search_png("assets/img/search.png"),
    menu_scene(menu_scene),
    back_rect_color(add_transition<kee::color>(kee::color(40, 40, 40))),
    color_music(add_transition<kee::color>(kee::color::white)),
    color_play(add_transition<kee::color>(kee::color::white)),
    color_edit(add_transition<kee::color>(kee::color::white)),
    color_delete(add_transition<kee::color>(kee::color::white)),
    top_bar_frame(add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 0.f),
        dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::rel, 0.075f)
        ),
        false
    )),
    back_button(top_bar_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::beg, 0.f),
        pos(pos::type::rel, 0.f),
        dims(
            dim(dim::type::aspect, 1.f),
            dim(dim::type::rel, 1.f)
        ),
        false
    )),
    back_rect(back_button.ref.add_child<kee::ui::rect>(std::nullopt,
        back_rect_color.get(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0.f),
        true, std::nullopt, std::nullopt
    )),
    back_image(back_rect.ref.add_child<kee::ui::image>(std::nullopt,
        assets.arrow_png, kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.15f),
        true, ui::image::display::shrink_to_fit, false, false, 0.f
    )),
    search_bar(top_bar_frame.ref.add_child<kee::ui::rect>(std::nullopt,
        kee::color(12, 12, 12),
        pos(pos::type::beg, 0.f),
        pos(pos::type::rel, 0.f),
        dims(
            dim(dim::type::abs, 0.f),
            dim(dim::type::rel, 1.f)
        ),
        false, std::nullopt, std::nullopt
    )),
    search_button(top_bar_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::end, 0.f),
        pos(pos::type::rel, 0.f),
        dims(
            dim(dim::type::aspect, 1.f),
            dim(dim::type::rel, 1.f)
        ),
        false
    )),
    search_rect(search_button.ref.add_child<kee::ui::rect>(std::nullopt,
        kee::color(40, 40, 40),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0.f),
        true, std::nullopt, std::nullopt
    )),
    search_image(search_rect.ref.add_child<kee::ui::image>(std::nullopt,
        search_png, kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.2f),
        true, ui::image::display::shrink_to_fit, true, false, 0.f
    )),
    level_list_scrollable(add_child<kee::ui::scrollable>(std::nullopt,
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 0.075f),
        dims(
            dim(dim::type::rel, 0.5f),
            dim(dim::type::rel, 0.925f)
        ),
        false,
        pos(pos::type::rel, 0.f),
        dim(dim::type::rel, 0.03f),
        pos(pos::type::rel, 0.03f),
        dim(dim::type::rel, 0.97f)
    )),
    level_list_inner(level_list_scrollable.ref.add_scrollable_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_w, 0.01f),
        true
    )),
    selected_bg(add_child<kee::ui::rect>(std::nullopt,
        kee::color(25, 25, 25, 100),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.075f),
        dims(
            dim(dim::type::rel, 0.5f),
            dim(dim::type::rel, 0.925f)
        ),
        false, std::nullopt, std::nullopt
    )),
    selected_frame(selected_bg.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_w, 0.05f),
        true
    )),
    selected_image_frame(selected_frame.ref.add_child<kee::ui::rect>(std::nullopt,
        kee::color(50, 50, 50),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 0.15f),
            dim(dim::type::aspect, static_cast<float>(kee::window_h) / kee::window_w)
        ),
        false, std::nullopt, std::nullopt
    )),
    selected_text_frame(selected_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::end, 0.f),
        pos(pos::type::beg, 0.f),
        dims(
            dim(dim::type::rel, 0.8f),
            dim(dim::type::abs, 0.f)
        ),
        false
    )),
    selected_song_name_text(selected_text_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 0.f),
        ui::text_size(ui::text_size::type::rel_h, 0.6f),
        std::nullopt, false, assets.font_semi_bold, std::string(), false
    )),
    selected_song_artist_text(selected_text_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(200, 200, 200),
        pos(pos::type::rel, 0.f),
        pos(pos::type::end, 0.f),
        ui::text_size(ui::text_size::type::rel_h, 0.35f),
        std::nullopt, false, assets.font_regular, std::string(), false
    )),
    selected_level_name_text(selected_text_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(150, 150, 150),
        pos(pos::type::beg, 0.f),
        pos(pos::type::end, 0.f),
        ui::text_size(ui::text_size::type::rel_h, 0.35f),
        std::nullopt, false, assets.font_italic, std::string(), false
    )),
    selected_performance_outer_frame(selected_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.21f),
        kee::dims(
            dim(dim::type::rel, 0.15f),
            dim(dim::type::rel, 0.15f)
        ),
        true
    )),
    selected_performance_frame(selected_performance_outer_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        kee::border(border::type::rel_h, 0.15f),
        true
    )),
    selected_rating_rect(selected_performance_frame.ref.add_child<kee::ui::rect>(std::nullopt,
        kee::color(120, 120, 120),
        pos(pos::type::rel, 0.f),
        pos(pos::type::rel, 0.f),
        kee::dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::aspect, 0.32f)
        ),
        false, std::nullopt,
        ui::rect_roundness(ui::rect_roundness::type::rel_h, 0.5f, std::nullopt)
    )),
    selected_rating_frame(selected_rating_rect.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.45f),
        pos(pos::type::rel, 0.5f),
        kee::dims(
            dim(dim::type::abs, 0.f),
            dim(dim::type::rel, 1.f)
        ),
        true
    )),
    selected_rating_star_img(selected_rating_frame.ref.add_child<kee::ui::image>(std::nullopt,
        assets.star_png, kee::color::white,
        pos(pos::type::beg, 0.f),
        pos(pos::type::rel, 0.5f),
        kee::dims(
            dim(dim::type::aspect, 1.f),
            dim(dim::type::rel, 0.8f)
        ),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    selected_rating_text(selected_rating_star_img.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 1.25f),
        pos(pos::type::rel, 0.f),
        ui::text_size(ui::text_size::type::rel_h, 1.f),
        std::nullopt, false, assets.font_semi_bold, "1", false
    )),
    selected_progress_text_frame(selected_performance_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.f),
        pos(pos::type::end, 0.f),
        kee::dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::abs, 0.f)
        ),
        false
    )),
    selected_progress_text(selected_progress_text_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.9f),
        std::nullopt, true, assets.font_semi_bold, std::string(), true
    )),
    selected_info_frame(selected_frame.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.15f),
        pos(pos::type::rel, 0.35f),
        kee::dims(
            dim(dim::type::rel, 0.7f),
            dim(dim::type::rel, 0.45f)
        ),
        false
    )),
    selected_high_score_text(selected_info_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::beg, 0.f),
        pos(pos::type::rel, 0.f),
        ui::text_size(ui::text_size::type::rel_h, play::selected_info_text_size),
        std::nullopt, false, assets.font_semi_bold, "High Score", false
    )),
    selected_misses_text(selected_info_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::beg, 0.f),
        pos(pos::type::rel, 1.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, play::selected_info_text_size),
        std::nullopt, false, assets.font_semi_bold, "Misses", false
    )),
    selected_acc_text(selected_info_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::beg, 0.f),
        pos(pos::type::rel, 2.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, play::selected_info_text_size),
        std::nullopt, false, assets.font_semi_bold, "Accuracy", false
    )),
    selected_combo_text(selected_info_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::beg, 0.f),
        pos(pos::type::rel, 3.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, play::selected_info_text_size),
        std::nullopt, false, assets.font_semi_bold, "Combo", false
    )),
    selected_best_streak_text(selected_info_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::beg, 0.f),
        pos(pos::type::rel, 4.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, play::selected_info_text_size),
        std::nullopt, false, assets.font_semi_bold, "Best Streak", false
    )),
    selected_attempts_text(selected_info_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::beg, 0.f),
        pos(pos::type::rel, 5.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, play::selected_info_text_size),
        std::nullopt, false, assets.font_semi_bold, "Attempts", false
    )),
    selected_high_score(selected_info_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::end, 0.f),
        pos(pos::type::rel, 0.f),
        ui::text_size(ui::text_size::type::rel_h, play::selected_info_text_size),
        std::nullopt, false, assets.font_regular, "--", false
    )),
    selected_misses(selected_info_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::end, 0.f),
        pos(pos::type::rel, 1.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, play::selected_info_text_size),
        std::nullopt, false, assets.font_regular, "--", false
    )),
    selected_acc(selected_info_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::end, 0.f),
        pos(pos::type::rel, 2.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, play::selected_info_text_size),
        std::nullopt, false, assets.font_regular, "--", false
    )),
    selected_combo(selected_info_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::end, 0.f),
        pos(pos::type::rel, 3.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, play::selected_info_text_size),
        std::nullopt, false, assets.font_regular, "--", false
    )),
    selected_best_streak(selected_info_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::end, 0.f),
        pos(pos::type::rel, 4.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, play::selected_info_text_size),
        std::nullopt, false, assets.font_regular, "--", false
    )),
    selected_attempts(selected_info_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::end, 0.f),
        pos(pos::type::rel, 5.f / 6.f),
        ui::text_size(ui::text_size::type::rel_h, play::selected_info_text_size),
        std::nullopt, false, assets.font_regular, "0", false
    )),
    button_music(selected_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.9f),
        dims(
            dim(dim::type::rel, 0.05f),
            dim(dim::type::aspect, 1.f)
        ),
        true
    )),
    button_play(selected_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.1f + 0.8f / 3),
        pos(pos::type::rel, 0.9f),
        dims(
            dim(dim::type::rel, 0.05f),
            dim(dim::type::aspect, 1.f)
        ),
        true
    )),
    button_edit(selected_frame.ref.add_child<kee::ui::button>(std::nullopt,
            pos(pos::type::rel, 0.1f + 2 * 0.8f / 3),
        pos(pos::type::rel, 0.9f),
        dims(
            dim(dim::type::rel, 0.05f),
            dim(dim::type::aspect, 1.f)
        ),
        true
    )),
    button_delete(selected_frame.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::rel, 0.9f),
        pos(pos::type::rel, 0.9f),
        dims(
            dim(dim::type::rel, 0.05f),
            dim(dim::type::aspect, 1.f)
        ),
        true
    )),
    img_music(button_music.ref.add_child<kee::ui::image>(std::nullopt,
        menu_scene.music_png, kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0.f),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    img_play(button_play.ref.add_child<kee::ui::image>(std::nullopt,
        assets.play_png, kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0.f),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    img_edit(button_edit.ref.add_child<kee::ui::image>(std::nullopt,
        menu_scene.edit_png, kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0.f),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    img_delete(button_delete.ref.add_child<kee::ui::image>(std::nullopt,
        assets.exit_png, kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0.f),
        true, ui::image::display::shrink_to_fit, false, false, 0.0f
    )),
    text_music(button_music.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 1.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.5f),
        std::nullopt, true, assets.font_regular, "MUSIC", false
    )),
    text_play(button_play.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 1.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.5f),
        std::nullopt, true, assets.font_regular, "PLAY", false
    )),
    text_edit(button_edit.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 1.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.5f),
        std::nullopt, true, assets.font_regular, "EDIT", false
    )),
    text_delete(button_delete.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 1.5f),
        ui::text_size(ui::text_size::type::rel_h, 0.5f),
        std::nullopt, true, assets.font_regular, "DELETE", false
    )),
    should_destruct_flag(false)
{
    const float back_rect_w = back_rect.ref.get_raw_rect().width;
    const float search_rect_x = search_rect.ref.get_raw_rect().x;

    search_bar.ref.x.val = back_rect_w;
    std::get<kee::dims>(search_bar.ref.dimensions).w.val = search_rect_x - back_rect_w;
    std::get<kee::dims>(selected_text_frame.ref.dimensions).h.val = selected_image_frame.ref.get_raw_rect().height;

    std::size_t selected_found = 0;
    std::size_t level_list_i = 0;

    float level_ui_y_first = 0.f;
    float level_ui_y_last = 0.f;
    float level_ui_height = 0.f;
    for (const auto& [beatmap_id, ui_assets] : assets.play_assets)
    {
        const bool is_analyzer_playing = (beatmap_id == analyzer_beatmap_id);
        if (is_analyzer_playing)
        {
            level_list_selected_id = beatmap_id;
            selected_found++;
        }

        const auto [inserted_it, _] = level_list.emplace(beatmap_id, level_list_inner.ref.add_child<level_ui>(std::nullopt,
            pos(pos::type::rel, 0.5f),
            pos(pos::type::rel, static_cast<float>(level_list_i) * 0.11f + 0.05f),
            dims(
                dim(dim::type::rel, 1.f),
                dim(dim::type::rel, 0.1f)
            ),
            true, is_analyzer_playing, *this, ui_assets, beatmap_id
        ));

        if (level_list_i == 0)
        {
            level_ui_y_first = inserted_it->second.ref.get_raw_rect().y;
            level_ui_height = inserted_it->second.ref.get_raw_rect().height;
        }
        else if (level_list_i == assets.play_assets.size() - 1)
            level_ui_y_last = inserted_it->second.ref.get_raw_rect().y;

        level_list_i++;
    }

    if (selected_found != 1)
        throw std::runtime_error("Music analyzer's beatmap is malformed");

    set_selected_ui(level_list_selected_id);

    const raylib::Rectangle level_list_scrollable_rect = level_list_scrollable.ref.scroll_frame_ui.ref.get_raw_rect();
    const float abs_margin = level_list_inner.ref.get_raw_rect().x - level_list_scrollable_rect.x;
    const float level_list_back_end_y = level_ui_y_last + level_ui_height;
    const float level_list_h = level_list_back_end_y - level_ui_y_first;

    const float abs_scrollable_h = level_list_h + 2 * abs_margin;
    const float rel_scrollable_h = abs_scrollable_h / level_list_scrollable_rect.height;
    level_list_scrollable.ref.set_scrollable_rel_h(rel_scrollable_h);

    back_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            back_rect_color.set(std::nullopt, kee::color(30, 30, 30), 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            back_rect_color.set(std::nullopt, kee::color(40, 40, 40), 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    back_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        should_destruct_flag = true;
    };

    button_music.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            color_music.set(std::nullopt, kee::color(150, 150, 150), 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            color_music.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    button_music.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        /* TODO: impl */
    };

    button_play.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            color_play.set(std::nullopt, kee::color(150, 150, 150), 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            color_play.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    button_play.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        this->game_ref.scene_manager.request_scene_switch([&]() {
            return game_ref.make_scene<kee::scene::beatmap>(level_list_selected_id, beatmap_dir_info(assets.play_assets.at(level_list_selected_id).beatmap_dir_path));
        });
    };

    button_edit.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            color_edit.set(std::nullopt, kee::color(150, 150, 150), 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            color_edit.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    button_edit.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        /* TODO: impl */
    };

    button_delete.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            color_delete.set(std::nullopt, kee::color(150, 150, 150), 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            color_delete.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    button_delete.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        /* TODO: impl */
    };

    float trns_time;
    if (!menu_scene.from_game_init && menu_scene.play_ui_immediate_flag)
    {
        menu_scene.play_ui_immediate_flag = false;
        trns_time = 0.f;
    }
    else
        trns_time = 0.75f;

    menu_scene.k_text_alpha.set(std::nullopt, 0.f, trns_time, kee::transition_type::exp);
    menu_scene.edit_text_alpha.set(std::nullopt, 0.f, trns_time, kee::transition_type::exp);
    menu_scene.play_text_alpha.set(std::nullopt, 0.f, trns_time, kee::transition_type::exp);
    menu_scene.browse_text_alpha.set(std::nullopt, 0.f, trns_time, kee::transition_type::exp);

    assert(menu_scene.opening_trns.has_value());
    menu_scene.opening_trns.value().k_rect_alpha.set(std::nullopt, 0.f, trns_time, kee::transition_type::exp);
    menu_scene.opening_trns.value().e1_text_alpha.set(std::nullopt, 0.f, trns_time, kee::transition_type::exp);
    menu_scene.opening_trns.value().e1_rect_alpha.set(std::nullopt, 0.f, trns_time, kee::transition_type::exp);
    menu_scene.opening_trns.value().e2_text_alpha.set(std::nullopt, 0.f, trns_time, kee::transition_type::exp);
    menu_scene.opening_trns.value().e2_rect_alpha.set(std::nullopt, 0.f, trns_time, kee::transition_type::exp);

    assert(menu_scene.music_trns.has_value());
    menu_scene.music_trns.value().slider_alpha.set(std::nullopt, 0.f, trns_time, kee::transition_type::exp);
    menu_scene.music_trns.value().pause_play_color.set(std::nullopt, kee::color::blank, trns_time, kee::transition_type::exp);
    menu_scene.music_trns.value().step_l_color.set(std::nullopt, kee::color::blank, trns_time, kee::transition_type::exp);
    menu_scene.music_trns.value().step_r_color.set(std::nullopt, kee::color::blank, trns_time, kee::transition_type::exp);
    menu_scene.music_trns.value().setting_color.set(std::nullopt, kee::color::blank, trns_time, kee::transition_type::exp);
    menu_scene.music_trns.value().exit_color.set(std::nullopt, kee::color::blank, trns_time, kee::transition_type::exp);
    menu_scene.music_trns.value().song_ui_alpha.set(std::nullopt, 0.f, trns_time, kee::transition_type::exp);
}

play::~play()
{
    menu_scene.e1_scale.set(1.0f);
    menu_scene.k_text_alpha.set(std::nullopt, 255.f, 0.75f, kee::transition_type::exp);

    assert(menu_scene.opening_trns.has_value());
    menu_scene.opening_trns.value().k_rect_alpha.set(std::nullopt, 255.f, 0.75f, kee::transition_type::exp);
    menu_scene.opening_trns.value().e1_text_alpha.set(std::nullopt, 255.f, 0.75f, kee::transition_type::exp);
    menu_scene.opening_trns.value().e1_rect_alpha.set(std::nullopt, 255.f, 0.75f, kee::transition_type::exp);
    menu_scene.opening_trns.value().e2_text_alpha.set(std::nullopt, 255.f, 0.75f, kee::transition_type::exp);
    menu_scene.opening_trns.value().e2_rect_alpha.set(std::nullopt, 255.f, 0.75f, kee::transition_type::exp);

    assert(menu_scene.music_trns.has_value());
    menu_scene.music_trns.value().slider_alpha.set(std::nullopt, 255.f, 0.75f, kee::transition_type::exp);
    menu_scene.music_trns.value().pause_play_color.set(std::nullopt, kee::color::white, 0.75f, kee::transition_type::exp);
    menu_scene.music_trns.value().step_l_color.set(std::nullopt, kee::color::white, 0.75f, kee::transition_type::exp);
    menu_scene.music_trns.value().step_r_color.set(std::nullopt, kee::color::white, 0.75f, kee::transition_type::exp);
    menu_scene.music_trns.value().setting_color.set(std::nullopt, kee::color::white, 0.75f, kee::transition_type::exp);
    menu_scene.music_trns.value().exit_color.set(std::nullopt, kee::color::white, 0.75f, kee::transition_type::exp);
    menu_scene.music_trns.value().song_ui_alpha.set(std::nullopt, 255.f, 0.75f, kee::transition_type::exp);
}

bool play::should_destruct() const
{
    return should_destruct_flag;
}

void play::set_selected_level(std::size_t beatmap_id)
{
    if (beatmap_id == level_list_selected_id)
        return;

    level_list.at(level_list_selected_id).ref.unselect();
    level_list.at(beatmap_id).ref.select();

    set_selected_ui(beatmap_id);
    level_list_selected_id = beatmap_id;
}

void play::update_element([[maybe_unused]] float dt)
{
    back_rect.ref.color = back_rect_color.get();

    img_music.ref.color = color_music.get();
    img_play.ref.color = color_play.get();
    img_edit.ref.color = color_edit.get();
    img_delete.ref.color = color_delete.get();

    text_music.ref.color = color_music.get();
    text_play.ref.color = color_play.get();
    text_edit.ref.color = color_edit.get();
    text_delete.ref.color = color_delete.get();
}

void play::set_selected_ui(std::size_t beatmap_id)
{
    const level_ui_assets& ui_assets = assets.play_assets.at(beatmap_id);
    if (ui_assets.img.has_value())
        selected_image.emplace(selected_image_frame.ref.add_child<kee::ui::image>(std::nullopt,
            ui_assets.img.value(), 
            kee::color::white,
            pos(pos::type::rel, 0.5f),
            pos(pos::type::rel, 0.5f),
            border(border::type::abs, 0),
            true, ui::image::display::shrink_to_fit, false, false, 0.0f
        ));
    else
        selected_image.reset();

    selected_song_name_text.ref.set_string(ui_assets.song_name);
    selected_song_artist_text.ref.set_string(ui_assets.song_artist);
    selected_song_artist_text.ref.refresh_ui();

    selected_level_name_text.ref.set_string(" - " + ui_assets.mapper + "'s " + ui_assets.level_name);
    selected_level_name_text.ref.x.val = selected_song_artist_text.ref.get_raw_rect().width;

    selected_rating_star_img.ref.x.val = selected_rating_star_img.ref.get_raw_rect().width / 2;
    selected_progress_text.ref.set_string(ui_assets.best.has_value() ? std::to_string(ui_assets.best.value().high_score) : "0");
    std::get<kee::dims>(selected_rating_frame.ref.dimensions).w.val = selected_rating_star_img.ref.get_raw_rect().width + selected_rating_text.ref.get_raw_rect().width;
    std::get<kee::dims>(selected_progress_text_frame.ref.dimensions).h.val = selected_performance_frame.ref.get_raw_rect().height - selected_rating_rect.ref.get_raw_rect().height;

    selected_high_score.ref.set_string(ui_assets.best.has_value() ? std::to_string(ui_assets.best.value().high_score) : "--");
    selected_misses.ref.set_string(ui_assets.best.has_value() ? std::to_string(ui_assets.best.value().misses) : "--");
    selected_acc.ref.set_string(ui_assets.best.has_value() ? std::format("{:.2f}", ui_assets.best.value().acc) : "--");
    selected_combo.ref.set_string(ui_assets.best.has_value() ? std::format("{}/{}", ui_assets.best.value().combo, ui_assets.total_combo) : "--");
    selected_best_streak.ref.set_string(ui_assets.best.has_value() ? std::to_string(ui_assets.best.value().best_streak) : "--");
    selected_attempts.ref.set_string(std::to_string(ui_assets.attempt_count));
}

menu::menu(const kee::scene::required& reqs, bool from_game_init, std::optional<std::size_t> start_beatmap_id) :
    kee::scene::base(reqs),
    edit_png("assets/img/edit.png"),
    music_png("assets/img/music.png"),
    from_game_init(from_game_init),
    play_ui_immediate_flag(true),
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
    analyzer(*this, start_beatmap_id),
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
        std::nullopt, false, assets.font_semi_bold, std::string(), false
    )),
    music_artist_text(music_info_text_frame.ref.add_child<kee::ui::text>(std::nullopt,
        kee::color(255, 255, 255, 0),
        pos(pos::type::beg, 0),
        pos(pos::type::end, 0),
        ui::text_size(ui::text_size::type::rel_h, 0.35f),
        std::nullopt, false, assets.font_regular, std::string(), true
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
        if (!music_trns.has_value() || play_ui.has_value())
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
        default:
            break;
        }
    };

    k_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (!music_trns.has_value())
            return;

        analyzer.pause();
        game_ref.scene_manager.request_scene_switch([&]() {
            return game_ref.make_scene<kee::scene::editor::root>(std::nullopt);
        });
    };

    e1_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (!music_trns.has_value() || play_ui.has_value())
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
        default:
            break;
        }
    };

    e1_button.ref.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (!music_trns.has_value())
            return;

        play_ui.emplace(add_child<play>(3, *this, analyzer.get_beatmap_id()));
    };

    e2_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        if (!music_trns.has_value() || play_ui.has_value())
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
        default:
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

    set_menu_level();

    float k_text_alpha_time;
    if (!from_game_init)
    {
        k_text_alpha_time = 0.f;

        opening_trns.emplace(*this);
        music_trns.emplace(*this);
        analyzer.play();
    }
    else
        k_text_alpha_time = 2.0f;

    if (start_beatmap_id.has_value())
        e1_button.ref.on_click_l(magic_enum::containers::bitset<kee::mods>());
    else
        k_text_alpha.set(std::nullopt, 255.0f, k_text_alpha_time, kee::transition_type::lin);
    
    analyzer.set_volume(1.f);
}

void menu::set_menu_level()
{
    const level_ui_assets& ui_assets = assets.play_assets.at(analyzer.get_beatmap_id());
    if (ui_assets.img.has_value())
        music_cover_art.emplace(music_cover_art_frame.ref.add_child<kee::ui::image>(std::nullopt,
            ui_assets.img.value(), 
            kee::color(255, 255, 255, 0),
            pos(pos::type::rel, 0.5f),
            pos(pos::type::rel, 0.5f),
            border(border::type::abs, 0),
            true, ui::image::display::shrink_to_fit, false, false, 0.0f
        ));
    else
        music_cover_art.reset();

    music_name_text.ref.set_string(ui_assets.song_name);
    music_artist_text.ref.set_string(ui_assets.song_artist);
}

void menu::update_element(float dt)
{
    if (scene_time >= 3.0f && !opening_trns.has_value())
        opening_trns.emplace(*this);
    else
        scene_time += dt;

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
        if (music_cover_art.has_value())
            music_cover_art.value().ref.color.a = music_trns.value().song_ui_alpha.get();
    }

    if (play_ui.has_value() && play_ui.value().ref.should_destruct())
        play_ui.reset();

    k_text.ref.color.a = k_text_alpha.get();
    edit_text.ref.color.a = edit_text_alpha.get();
    k_rect.ref.outline.value().color.a = opening_trns.has_value() ? opening_trns.value().k_rect_alpha.get() : 0.f;
    k_button.ref.x.val = opening_trns.has_value() ? opening_trns.value().k_rect_x.get() : 0.5f;
    std::get<kee::dims>(k_button.ref.dimensions).h.val = 0.25f * k_scale.get();

    e1_text.ref.color.a = opening_trns.has_value() ? opening_trns.value().e1_text_alpha.get() : 0.f;
    play_text.ref.color.a = play_text_alpha.get();
    e1_rect.ref.outline.value().color.a = opening_trns.has_value() ? opening_trns.value().e1_rect_alpha.get() : 0.f;
    std::get<kee::dims>(e1_button.ref.dimensions).h.val = 0.25f * e1_scale.get();

    e2_text.ref.color.a = opening_trns.has_value() ? opening_trns.value().e2_text_alpha.get() : 0.f;
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