#include "kee/ui/image.hpp"

namespace kee {
namespace ui {

image_texture::image_texture(const std::filesystem::path& img_path) :
    texture(img_path.string())
{
    texture.GenMipmaps();
    texture.SetFilter(TextureFilter::TEXTURE_FILTER_TRILINEAR);
}

image::image(
    const kee::ui::base::required& reqs,
    const kee::ui::image_texture& img_texture,
    const raylib::Color& color,
    kee::pos x, 
    kee::pos y,
    const std::variant<kee::dims, kee::border>& dimensions,
    bool centered,
    bool flip_horizontal,
    bool flip_vertical
) :
    kee::ui::base(reqs, x, y, dimensions, centered),
    flip_horizontal(flip_horizontal),
    flip_vertical(flip_vertical),
    img_texture_ref(img_texture)
{ 
    set_opt_color(color);
}

void image::set_image(const kee::ui::image_texture& new_img_texture)
{
    img_texture_ref = new_img_texture;
}

void image::render_element_behind_children() const
{
    const raylib::Rectangle raw_rect = get_raw_rect();
    const raylib::Texture& img_texture = img_texture_ref.get().texture;

    const float scale = (img_texture.width * raw_rect.height >= img_texture.height * raw_rect.width)
        ? raw_rect.width / img_texture.width
        : raw_rect.height / img_texture.height;

    const raylib::Vector2 img_size = img_texture.GetSize();
    const raylib::Vector2 img_size_scaled = img_size * scale;

    raylib::Rectangle img_src(raylib::Vector2(0, 0), img_size);
    if (flip_horizontal)
        img_src.width *= -1;
    if (flip_vertical)
        img_src.height *= -1;
    
    const raylib::Rectangle img_dst(
        raw_rect.x + raw_rect.width / 2 - img_size_scaled.x / 2,
        raw_rect.y + raw_rect.height / 2 - img_size_scaled.y / 2,
        img_size_scaled.x,
        img_size_scaled.y
    );

    img_texture.Draw(img_src, img_dst, raylib::Vector2(0, 0), 0.0f, get_color_from_opt(get_opt_color()));
}

} // namespace ui
} // namespace kee