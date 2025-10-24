#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {

class image final : public kee::ui::base
{
public:
    image(
        const kee::ui::required& reqs,
        const kee::image_texture& img_texture,
        const raylib::Color& color,
        kee::pos x, 
        kee::pos y,
        const std::variant<kee::dims, kee::border>& dimensions,
        bool centered,
        bool flip_horizontal,
        bool flip_vertical,
        float rotation
    );

    void set_image(const kee::image_texture& new_img_texture);

    float rotation;

private:
    void render_element() const override;

    const bool flip_horizontal;
    const bool flip_vertical;

    std::reference_wrapper<const kee::image_texture> img_texture_ref;
};

} // namespace ui
} // namespace kee