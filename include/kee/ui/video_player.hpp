#pragma once

#include <avcpp/codeccontext.h>
#include <avcpp/formatcontext.h>
#include <avcpp/videorescaler.h>

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {

class video_player final : public kee::ui::base 
{
public:
    video_player(
        const kee::ui::required& reqs, 
        const std::filesystem::path& mp4_path,
        const kee::color& color_param,
        const kee::pos& x, 
        const kee::pos& y, 
        const std::variant<kee::dims, kee::border>& dimensions, 
        bool centered
    );

    void set_time(double sec);

private:
    static constexpr unsigned int frame_seek_tolerance = 10;

    void render_element() const override;

    std::optional<av::VideoFrame> get_next_frame();

    std::size_t video_stream_idx;
    double frame_rate;

    av::FormatContext video_input;
    av::VideoDecoderContext video_decoder;
    av::VideoRescaler video_rescaler;

    double curr_ts;
    raylib::Texture curr_texture;

    std::optional<av::VideoFrame> next_video_frame;
};

} // namespace ui
} // namespace kee