#include "kee/ui/video_player.hpp"

namespace kee {
namespace ui {

/* WISHLIST: video texture/decoder init on every ctor instance, maybe store assets like `kee::image_texture` does */

video_player::video_player(
    const kee::ui::required& reqs,
    const std::filesystem::path& mp4_path,
    const kee::color& color_param,
    const kee::pos& x, 
    const kee::pos& y, 
    const std::variant<kee::dims, kee::border>& dimensions, 
    bool centered
) :
    kee::ui::base(reqs, x, y, dimensions, centered)
{
    color = color_param;

    video_input.openInput(mp4_path.string());
    video_input.findStreamInfo();

    std::optional<std::size_t> opt_video_stream_idx = std::nullopt;
    av::Stream video_stream;
    for (std::size_t i = 0; i < video_input.streamsCount(); i++) 
    {
        const av::Stream stream = video_input.stream(i);
        if (stream.mediaType() == AVMEDIA_TYPE_VIDEO) 
        {
            opt_video_stream_idx = i;
            video_stream = stream;

            frame_rate = stream.frameRate().getDouble();
            break;
        }
    }

    if (!opt_video_stream_idx.has_value()) 
        throw std::runtime_error("Can't find video stream");
    
    video_stream_idx = opt_video_stream_idx.value();
    if (video_stream.isValid()) 
    {
        video_decoder = av::VideoDecoderContext(video_stream);

        av::Codec codec = av::findDecodingCodec(video_decoder.raw()->codec_id);
        video_decoder.setCodec(codec);
        video_decoder.setRefCountedFrames(true);
        video_decoder.open({{"threads", "1"}});
    }
    
    curr_texture.width = video_decoder.width();
    curr_texture.height = video_decoder.height();
    curr_texture.format = PixelFormat::PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    curr_texture.mipmaps = 1;
    /**
     * NOTE: `raylib-cpp` has no binding for `rlLoadTexture`
     */
    curr_texture.id = rlLoadTexture(nullptr, curr_texture.width, curr_texture.height, curr_texture.format, curr_texture.mipmaps);

    video_rescaler = av::VideoRescaler(video_decoder.width(), video_decoder.height(), video_decoder.pixelFormat(), video_decoder.width(), video_decoder.height(), AV_PIX_FMT_RGB24);                                                                                                                                                                               
    const std::optional<av::VideoFrame> decoded_frame = get_next_frame();
    if (!decoded_frame.has_value())
        throw std::runtime_error("Given video has no frames.");

    curr_ts = decoded_frame.value().pts().seconds();
    curr_texture.Update(decoded_frame.value().data(0));

    next_video_frame = get_next_frame();
}

void video_player::set_time(double sec)
{
    sec = std::clamp(sec, 0., static_cast<double>(video_input.duration()));

    const bool less_than_next_ts = !next_video_frame.has_value() || sec <= next_video_frame.value().pts().seconds();
    if (curr_ts <= sec && less_than_next_ts)
        return;

    bool should_seek = false;
    if (sec < curr_ts)
        should_seek = true;
    else
    {
        const unsigned int frames_to_shift = 1 + static_cast<unsigned int>((sec - next_video_frame.value().pts().seconds()) * frame_rate);
        should_seek = frames_to_shift > video_player::frame_seek_tolerance;
    }

    if (should_seek)
    {
        const av::Stream video_stream = video_input.stream(video_stream_idx);
        const int64_t target_ts = static_cast<int64_t>(sec / video_stream.timeBase()());

        video_input.seek(target_ts, static_cast<int>(video_stream_idx), AVSEEK_FLAG_BACKWARD);
        video_input.flush();

        std::optional<av::VideoFrame> opt_curr_video_frame = get_next_frame();
        if (!opt_curr_video_frame.has_value())
            throw std::runtime_error("Keyframe not decodable!"); // TODO SOON: don't think this is correct just yet

        av::VideoFrame curr_video_frame = std::move(opt_curr_video_frame.value());
        next_video_frame = get_next_frame();
        while (next_video_frame.has_value() && next_video_frame.value().pts().seconds() < sec)
        {
            curr_video_frame = std::move(next_video_frame.value());
            next_video_frame = get_next_frame();
        }

        curr_texture.Update(curr_video_frame.data(0));
    }
    else 
    {
        while (next_video_frame.has_value() && sec > next_video_frame.value().pts().seconds())
        {
            curr_ts = next_video_frame.value().pts().seconds();
            curr_texture.Update(next_video_frame.value().data(0));

            next_video_frame = get_next_frame();
        }
    }
}

void video_player::render_element() const
{
    const raylib::Rectangle raw_rect = get_raw_rect();
    const float scale = (curr_texture.width * raw_rect.height >= curr_texture.height * raw_rect.width)
        ? raw_rect.width / curr_texture.width
        : raw_rect.height / curr_texture.height;

    const raylib::Vector2 tex_size = curr_texture.GetSize();
    const raylib::Vector2 tex_size_scaled = tex_size * scale;

    const raylib::Rectangle tex_src(raylib::Vector2(0, 0), tex_size);
    const raylib::Rectangle tex_dst(
        raw_rect.x + raw_rect.width / 2,
        raw_rect.y + raw_rect.height / 2,
        tex_size_scaled.x,
        tex_size_scaled.y
    );

    curr_texture.Draw(tex_src, tex_dst, tex_size_scaled / 2, 0.0f, color.raylib());
}

std::optional<av::VideoFrame> video_player::get_next_frame()
{
    while (const av::Packet packet = video_input.readPacket()) 
    {
        if (static_cast<std::size_t>(packet.streamIndex()) != video_stream_idx)
            continue;

        const av::VideoFrame frame = video_decoder.decode(packet);
        if (!frame)
            continue;

        av::VideoFrame res(AV_PIX_FMT_RGB24, video_decoder.width(), video_decoder.height());
        video_rescaler.rescale(res, frame);
        return res;
    }

    return std::nullopt;
}
    
} // namespace ui
} // namespace kee