#include "kee/ui/image.hpp"

#include "kee/game.hpp"

namespace kee {
namespace ui {

image::image(
    const kee::ui::required& reqs,
    const raylib::Image& img,
    const kee::color& color_param,
    const kee::pos& x, 
    const kee::pos& y,
    const std::variant<kee::dims, kee::border>& dimensions,
    bool centered,
    image::display display_setting,
    bool flip_horizontal,
    bool flip_vertical,
    float rotation
) :
    kee::ui::base(reqs, x, y, dimensions, centered),
    rotation(rotation),
    display_setting(display_setting),
    flip_horizontal(flip_horizontal),
    flip_vertical(flip_vertical),
    texture(img)
{
    texture.GenMipmaps();
    texture.SetFilter(TextureFilter::TEXTURE_FILTER_TRILINEAR);

    color = color_param;
}

void image::set_image(const raylib::Image& new_img)
{
    texture = new_img;
    texture.GenMipmaps();
    texture.SetFilter(TextureFilter::TEXTURE_FILTER_TRILINEAR);
}

void image::render_element() const
{
    const raylib::Rectangle raw_rect = get_raw_rect();

    float scale;
    if (texture.width * raw_rect.height >= texture.height * raw_rect.width)
    {
        scale = (display_setting == image::display::shrink_to_fit) 
            ? raw_rect.width / texture.width
            : raw_rect.height / texture.height;
    }
    else
    {
        scale = (display_setting == image::display::shrink_to_fit) 
            ? raw_rect.height / texture.height
            : raw_rect.width / texture.width;
    }

    const raylib::Vector2 img_size = texture.GetSize();
    const raylib::Vector2 img_size_scaled = img_size * scale;

    raylib::Rectangle img_src(raylib::Vector2(0, 0), img_size);
    if (flip_horizontal)
        img_src.width *= -1;
    if (flip_vertical)
        img_src.height *= -1;
    
    const raylib::Rectangle img_dst(
        raw_rect.x + raw_rect.width / 2,
        raw_rect.y + raw_rect.height / 2,
        img_size_scaled.x,
        img_size_scaled.y
    );

    game_ref.scissor_mode.push(get_raw_rect());
    texture.Draw(img_src, img_dst, img_size_scaled / 2, rotation, color.raylib());
    game_ref.scissor_mode.pop();
}

} // namespace ui
} // namespace kee