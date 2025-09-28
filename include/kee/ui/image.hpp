#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {

class image_texture
{
public:
    image_texture(const std::filesystem::path& img_path);

    raylib::Texture texture;
};

class image final : public kee::ui::base
{
public:
    image(
        const kee::ui::base::required& reqs,
        const kee::ui::image_texture& img_texture,
        const raylib::Color& color,
        kee::pos x, 
        kee::pos y,
        const std::variant<kee::dims, kee::border>& dimensions,
        bool centered,
        bool flip_horizontal,
        bool flip_vertical
    );

    void set_image(const kee::ui::image_texture& new_img_texture);

private:
    void render_element_behind_children() const override;

    const bool flip_horizontal;
    const bool flip_vertical;

    std::reference_wrapper<const kee::ui::image_texture> img_texture_ref;
};

} // namespace ui
} // namespace kee