#include "kee/scene/menu.hpp"

namespace kee {
namespace scene {

music_analyzer::music_analyzer(const std::filesystem::path& music_path) :
    wave(music_path.string()),
    frame_cursor(0)
{ 
    SetAudioStreamBufferSizeDefault(music_analyzer::frames_per_refresh);

    audio_stream = LoadAudioStream(music_analyzer::sample_rate, music_analyzer::bit_depth, music_analyzer::channels);
    PlayAudioStream(audio_stream);

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
        for (int i = 0; i < music_analyzer::fft_resolution; i++)
        {
            unsigned int cursor = frame_cursor + i;

            /* TODO: warning */
            if (cursor >= wave.frameCount)
            {
                frame_cursor = 0;
                break;
            }

            for (unsigned int ch = 0; ch < music_analyzer::channels; ch++)
                to_submit[i * music_analyzer::channels + ch] = samples[cursor * music_analyzer::channels + ch];
        }

        UpdateAudioStream(audio_stream, to_submit.data(), music_analyzer::frames_per_refresh);
        frame_cursor += music_analyzer::frames_per_refresh;

        /* TODO: perform FFT analysis */
    }
}

/*

REFERENCE:

#include "raylib.h"

#include "raymath.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <print>

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#define STEREO                           2
#define SAMPLE_RATE                    48000
#define SAMPLE_RATE_F                  48000.0f
#define FFT_WINDOW_SIZE                16384
#define BUFFER_SIZE                    64
#define PER_SAMPLE_BIT_DEPTH           16
#define AUDIO_STREAM_RING_BUFFER_SIZE  (4096)
#define EFFECTIVE_SAMPLE_RATE          (SAMPLE_RATE_F*0.5f)
#define WINDOW_TIME                    ((double)FFT_WINDOW_SIZE/(double)EFFECTIVE_SAMPLE_RATE)
#define FFT_HISTORICAL_SMOOTHING_DUR   2.0f
#define MIN_DECIBELS                   (-100.0f) // https://developer.mozilla.org/en-US/docs/Web/API/AnalyserNode/minDecibels
#define MAX_DECIBELS                   (-33.0f)  // https://developer.mozilla.org/en-US/docs/Web/API/AnalyserNode/maxDecibels
#define INVERSE_DECIBEL_RANGE          (1.0f/(MAX_DECIBELS - MIN_DECIBELS))
#define DB_TO_LINEAR_SCALE             (20.0f/2.302585092994046f)
#define SMOOTHING_TIME_CONSTANT        0.8f // https://developer.mozilla.org/en-US/docs/Web/API/AnalyserNode/smoothingTimeConstant
#define TEXTURE_HEIGHT                 1
#define FFT_ROW                        0
#define UNUSED_CHANNEL                 0.0f

typedef struct FFTComplex { float real, imaginary; } FFTComplex;

typedef struct FFTData {
    FFTComplex *spectrum;
    FFTComplex *workBuffer;
    float *prevMagnitudes;
    float (*fftHistory)[BUFFER_SIZE];
    int fftHistoryLen;
    int historyPos;
    double lastFftTime;
    float tapbackPos;
} FFTData;

static void CaptureFrame(FFTData *fftData, const float *audioSamples);
static void RenderFrame(const FFTData *fftData, Image *fftImage);
static void CooleyTukeyFFTSlow(FFTComplex *spectrum, int n);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //-----------------------------------------------------------------------------------     ---
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [audio] example - spectrum visualizer");

    Image fftImage = GenImageColor(BUFFER_SIZE, TEXTURE_HEIGHT, WHITE);
    Texture2D fftTexture = LoadTextureFromImage(fftImage);
    RenderTexture2D bufferA = LoadRenderTexture(screenWidth, screenHeight);
    Vector2 iResolution = { (float)screenWidth, (float)screenHeight };

    Shader shader = LoadShader(0, TextFormat("C:/Users/suman/source/personal/test_cpp/resources/shaders/glsl%i/fft.fs", GLSL_VERSION));

    int iResolutionLocation = GetShaderLocation(shader, "iResolution");
    int iChannel0Location = GetShaderLocation(shader, "iChannel0");
    SetShaderValue(shader, iResolutionLocation, &iResolution, SHADER_UNIFORM_VEC2);
    SetShaderValueTexture(shader, iChannel0Location, fftTexture);

    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(AUDIO_STREAM_RING_BUFFER_SIZE);

    // WARNING: Memory out-of-bounds on PLATFORM_WEB
    Wave wav = LoadWave("C:/Users/suman/source/personal/test_cpp/resources/song.mp3");
    WaveFormat(&wav, SAMPLE_RATE, PER_SAMPLE_BIT_DEPTH, STEREO);

    AudioStream audioStream = LoadAudioStream(SAMPLE_RATE, PER_SAMPLE_BIT_DEPTH, STEREO);
    PlayAudioStream(audioStream);

    int fftHistoryLen = (int)ceilf(FFT_HISTORICAL_SMOOTHING_DUR/WINDOW_TIME) + 1;

    FFTData fft = {
        .spectrum = (FFTComplex*)RL_CALLOC(sizeof(FFTComplex), FFT_WINDOW_SIZE),
        .workBuffer = (FFTComplex*)RL_CALLOC(sizeof(FFTComplex), FFT_WINDOW_SIZE),
        .prevMagnitudes = (float*)RL_CALLOC(BUFFER_SIZE, sizeof(float)),
        .fftHistory = (float(*)[BUFFER_SIZE])RL_CALLOC(fftHistoryLen, sizeof(float[BUFFER_SIZE])),
        .fftHistoryLen = fftHistoryLen,
        .historyPos = 0,
        .lastFftTime = 0.0,
        .tapbackPos = 0.01f
    };

    int wavCursor = 0;
    const short *wavPCM16 = (short*)wav.data;

    short chunkSamples[FFT_WINDOW_SIZE * 2] = { 0 };
    float audioSamples[FFT_WINDOW_SIZE] = { 0 };

    SetTargetFPS(60);
    //----------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        while (IsAudioStreamProcessed(audioStream))
        {
            for (int i = 0; i < FFT_WINDOW_SIZE; i++)
            {
                int cursor = wavCursor + i;
                int left = (wav.channels == 2)? wavPCM16[cursor*2 + 0] : wavPCM16[cursor];
                int right = (wav.channels == 2)? wavPCM16[cursor*2 + 1] : left;
                chunkSamples[i*2 + 0] = left;
                chunkSamples[i*2 + 1] = right;

                // this is to loop the audio
                //if (++wavCursor >= wav.frameCount) wavCursor = 0;
            }

            wavCursor += AUDIO_STREAM_RING_BUFFER_SIZE;
            UpdateAudioStream(audioStream, chunkSamples, AUDIO_STREAM_RING_BUFFER_SIZE);

            for (int i = 0; i < FFT_WINDOW_SIZE; i++) audioSamples[i] = (chunkSamples[i*2] + chunkSamples[i*2 + 1])*0.5f/32767.0f;
        }

        CaptureFrame(&fft, audioSamples);
        RenderFrame(&fft, &fftImage);
        UpdateTexture(fftTexture, fftImage.data);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginShaderMode(shader);
                SetShaderValueTexture(shader, iChannel0Location, fftTexture);
                DrawTextureRec(bufferA.texture,
                    Rectangle { 0, 0, (float)screenWidth, (float)-screenHeight },
                    Vector2 { 0, 0 }, WHITE);
            EndShaderMode();

        EndDrawing();
        //------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(shader);
    UnloadRenderTexture(bufferA);
    UnloadTexture(fftTexture);
    UnloadImage(fftImage);
    UnloadAudioStream(audioStream);
    UnloadWave(wav);
    CloseAudioDevice();

    RL_FREE(fft.spectrum);
    RL_FREE(fft.workBuffer);
    RL_FREE(fft.prevMagnitudes);
    RL_FREE(fft.fftHistory);

    CloseWindow();        // Close window and OpenGL context
    //----------------------------------------------------------------------------------

    return 0;
}

// Cooley–Tukey FFT https://en.wikipedia.org/wiki/Cooley%E2%80%93Tukey_FFT_algorithm#Data_reordering,_bit_reversal,_and_in-place_algorithms
static void CooleyTukeyFFTSlow(FFTComplex *spectrum, int n)
{
    int j = 0;
    for (int i = 1; i < n - 1; i++)
    {
        int bit = n >> 1;
        while (j >= bit)
        {
            j -= bit;
            bit >>= 1;
        }
        j += bit;
        if (i < j)
        {
            FFTComplex temp = spectrum[i];
            spectrum[i] = spectrum[j];
            spectrum[j] = temp;
        }
    }

    for (int len = 2; len <= n; len <<= 1)
    {
        float angle = -2.0f*PI/len;
        FFTComplex twiddleUnit = { cosf(angle), sinf(angle) };
        for (int i = 0; i < n; i += len)
        {
            FFTComplex twiddleCurrent = { 1.0f, 0.0f };
            for (int j = 0; j < len/2; j++)
            {
                FFTComplex even = spectrum[i + j];
                FFTComplex odd = spectrum[i + j + len/2];
                FFTComplex twiddledOdd = {
                    odd.real*twiddleCurrent.real - odd.imaginary*twiddleCurrent.imaginary,
                    odd.real*twiddleCurrent.imaginary + odd.imaginary*twiddleCurrent.real
                };

                spectrum[i + j].real = even.real + twiddledOdd.real;
                spectrum[i + j].imaginary = even.imaginary + twiddledOdd.imaginary;
                spectrum[i + j + len/2].real = even.real - twiddledOdd.real;
                spectrum[i + j + len/2].imaginary = even.imaginary - twiddledOdd.imaginary;

                float twiddleRealNext = twiddleCurrent.real*twiddleUnit.real - twiddleCurrent.imaginary*twiddleUnit.imaginary;
                twiddleCurrent.imaginary = twiddleCurrent.real*twiddleUnit.imaginary + twiddleCurrent.imaginary*twiddleUnit.real;
                twiddleCurrent.real = twiddleRealNext;
            }
        }
    }
}

static void CaptureFrame(FFTData *fftData, const float *audioSamples)
{
    for (int i = 0; i < FFT_WINDOW_SIZE; i++)
    {
        float x = (2.0f*PI*i)/(FFT_WINDOW_SIZE - 1.0f);
        float blackmanWeight  = 0.5f * (1.0f - cosf(x)); // https://en.wikipedia.org/wiki/Window_function#Blackman_window
        fftData->workBuffer[i].real = audioSamples[i]*blackmanWeight;
        fftData->workBuffer[i].imaginary = 0.0f;
    }

    CooleyTukeyFFTSlow(fftData->workBuffer, FFT_WINDOW_SIZE);
    memcpy(fftData->spectrum, fftData->workBuffer, sizeof(FFTComplex)*FFT_WINDOW_SIZE);

    float smoothedSpectrum[BUFFER_SIZE];

    for (int bin = 0; bin < BUFFER_SIZE; bin++)
    {
        float re = fftData->workBuffer[bin].real;
        float im = fftData->workBuffer[bin].imaginary;
        float linearMagnitude = sqrtf(re*re + im*im)/FFT_WINDOW_SIZE;

        float smoothedMagnitude = SMOOTHING_TIME_CONSTANT*fftData->prevMagnitudes[bin] + (1.0f - SMOOTHING_TIME_CONSTANT)*linearMagnitude;
        fftData->prevMagnitudes[bin] = smoothedMagnitude;

        float db = logf(fmaxf(smoothedMagnitude, 1e-40f))*DB_TO_LINEAR_SCALE;
        float normalized = (db - MIN_DECIBELS)*INVERSE_DECIBEL_RANGE;
        smoothedSpectrum[bin] = Clamp(normalized, 0.0f, 1.0f);
    }

    fftData->lastFftTime = GetTime();
    memcpy(fftData->fftHistory[fftData->historyPos], smoothedSpectrum, sizeof(smoothedSpectrum));
    fftData->historyPos = (fftData->historyPos + 1)%fftData->fftHistoryLen;
}

static void RenderFrame(const FFTData *fftData, Image *fftImage)
{
    double framesSinceTapback = floor(fftData->tapbackPos/WINDOW_TIME);
    framesSinceTapback = Clamp(framesSinceTapback, 0.0, fftData->fftHistoryLen - 1);

    int historyPosition = (fftData->historyPos - 1 - (int)framesSinceTapback)%fftData->fftHistoryLen;
    if (historyPosition < 0) historyPosition += fftData->fftHistoryLen;

    const float *amplitude = fftData->fftHistory[historyPosition];
    for (int bin = 0; bin < BUFFER_SIZE; bin++) ImageDrawPixel(fftImage, bin, FFT_ROW, ColorFromNormalized(Vector4 { amplitude[bin], UNUSED_CHANNEL, UNUSED_CHANNEL, UNUSED_CHANNEL }));
}

*/

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

void music_analyzer::resume()
{
    return ResumeAudioStream(audio_stream);
}

void music_analyzer::pause()
{
    return PauseAudioStream(audio_stream);
}

void music_analyzer::set_volume(float new_volume)
{
    const float new_volume_clamped = std::clamp(new_volume, 0.f, 1.f);
    SetAudioStreamVolume(audio_stream, new_volume_clamped);
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
    music_volume_multiplier(menu_scene.add_transition<float>(0.0f)),
    music_volume_trns_finished(false),
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
    music_slider(menu_scene.add_child<kee::ui::slider>(std::nullopt,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.895f),
        dims(
            dim(dim::type::rel, 1.f),
            dim(dim::type::rel, 0.01f)
        ),
        true, true
    )),
    pause_play(menu_scene.add_child<kee::ui::button>(std::nullopt,
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
    step_l(menu_scene.add_child<kee::ui::button>(std::nullopt,
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
    step_r(menu_scene.add_child<kee::ui::button>(std::nullopt,
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
    setting_exit_frame(menu_scene.add_child<kee::ui::base>(std::nullopt,
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
    music_volume_multiplier.set(std::nullopt, 2.0f, 2.0f, kee::transition_type::inv_exp);
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
            pause_play_color.set(std::nullopt, kee::color(255, 255, 255, 200), 0.5f, kee::transition_type::exp);
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
            step_l_color.set(std::nullopt, kee::color(255, 255, 255, 200), 0.5f, kee::transition_type::exp);
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
            step_r_color.set(std::nullopt, kee::color(255, 255, 255, 200), 0.5f, kee::transition_type::exp);
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
            setting_color.set(std::nullopt, kee::color(255, 255, 255, 200), 0.5f, kee::transition_type::exp);
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
            exit_color.set(std::nullopt, kee::color(255, 255, 255, 200), 0.5f, kee::transition_type::exp);
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

menu::menu(kee::game& game, kee::global_assets& assets, beatmap_dir_info&& beatmap_info) :
    kee::scene::base(game, assets),
    k_text_alpha(add_transition<float>(0.0f)),
    k_rect(add_child<kee::ui::rect>(std::nullopt,
        kee::color::blank,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.25f)
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
    e1_rect(add_child<kee::ui::rect>(std::nullopt,
        kee::color::blank,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.25f)
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
    e2_rect(add_child<kee::ui::rect>(std::nullopt,
        kee::color::blank,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 0.25f)
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
    music_cover_art_texture(beatmap_info.dir_state.has_image
        ? std::make_optional(beatmap_info.dir_state.path / beatmap_dir_state::standard_img_filename)
        : std::nullopt
    ),
    analyzer(beatmap_info.dir_state.path / beatmap_dir_state::standard_music_filename),
    music_time(0.f),
    song_ui_frame_outer(add_child<kee::ui::base>(std::nullopt,
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

    k_text_alpha.set(std::nullopt, 255.0f, 2.0f, kee::transition_type::lin);

    analyzer.set_volume(0.f);
}

void menu::update_element(float dt)
{
    analyzer.update();

    if (music_trns.has_value())
    {
        if (!music_trns.value().music_volume_trns_finished)
        {
            const float music_volume_multiplier = music_trns.value().music_volume_multiplier.get();
            analyzer.set_volume(0.01f * music_volume_multiplier);

            if (music_volume_multiplier >= 1.0f)
                music_trns.value().music_volume_trns_finished = true;
        }

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

    scene_time += dt;
    if (scene_time >= 3.0f && !opening_trns.has_value())
        opening_trns.emplace(*this);

    if (scene_time >= 3.5f && !music_trns.has_value())
    {
        music_trns.emplace(*this);
        analyzer.resume();
    }

    k_text.ref.color.a = k_text_alpha.get();
    k_rect.ref.outline.value().color.a = opening_trns.has_value() ? opening_trns.value().k_rect_alpha.get() : 0.f;
    k_rect.ref.x.val = opening_trns.has_value() ? opening_trns.value().k_rect_x.get() : 0.5f;

    e1_text.ref.color.a = opening_trns.has_value() ? opening_trns.value().e1_text_alpha.get() : 0.f;
    e1_rect.ref.outline.value().color.a = opening_trns.has_value() ? opening_trns.value().e1_rect_alpha.get() : 0.f;

    e2_text.ref.color.a = opening_trns.has_value() ? opening_trns.value().e2_text_alpha.get() : 0.f;
    e2_rect.ref.outline.value().color.a = opening_trns.has_value() ? opening_trns.value().e2_rect_alpha.get() : 0.f;
    e2_rect.ref.x.val = opening_trns.has_value() ? opening_trns.value().e2_rect_x.get() : 0.5f;
}

} // namespace scene
} // namespace kee