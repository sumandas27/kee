#include "kee/ui/video_player.hpp"

#include <avcpp/codeccontext.h>
#include <avcpp/formatcontext.h>
#include <avcpp/videorescaler.h>

namespace kee {
namespace ui {

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

    av::FormatContext input;
    input.openInput(mp4_path.string());
    input.findStreamInfo();

    std::optional<std::size_t> opt_video_stream_idx = std::nullopt;
    av::Stream video_stream;
    for (std::size_t i = 0; i < input.streamsCount(); i++) 
    {
        const av::Stream stream = input.stream(i);
        if (stream.mediaType() == AVMEDIA_TYPE_VIDEO) 
        {
            opt_video_stream_idx = i;
            video_stream = stream;
            break;
        }
    }

    if (!opt_video_stream_idx.has_value()) 
        throw std::runtime_error("Can't find video stream");
    
    std::size_t video_stream_idx = opt_video_stream_idx.value();
    av::VideoDecoderContext video_decoder;
    if (video_stream.isValid()) 
    {
        video_decoder = av::VideoDecoderContext(video_stream);

        av::Codec codec = av::findDecodingCodec(video_decoder.raw()->codec_id);
        video_decoder.setCodec(codec);
        video_decoder.setRefCountedFrames(true);
        video_decoder.open({{"threads", "1"}});
    }
    
    av::VideoRescaler rescaler(video_decoder.width(), video_decoder.height(), video_decoder.pixelFormat(), video_decoder.width(), video_decoder.height(), AV_PIX_FMT_RGB24);
    curr_texture.width = video_decoder.width();
    curr_texture.height = video_decoder.height();
    curr_texture.format = PixelFormat::PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    curr_texture.mipmaps = 1;
    /**
     * NOTE: `raylib-cpp` has no binding for `rlLoadTexture`
     */
    curr_texture.id = rlLoadTexture(nullptr, curr_texture.width, curr_texture.height, curr_texture.format, curr_texture.mipmaps);

    av::VideoFrame rgb_frame(AV_PIX_FMT_RGB24, video_decoder.width(), video_decoder.height());
    while (av::Packet packet = input.readPacket()) 
    {
        if (packet.streamIndex() != video_stream_idx)
            continue;

        av::VideoFrame frame = video_decoder.decode(packet);
        if (!frame)
            continue;

        rescaler.rescale(rgb_frame, frame);        
        curr_texture.Update(rgb_frame.data(0));
        break;
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
    
} // namespace ui
} // namespace kee