#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {

class image final : public kee::ui::base
{
public:
    enum class display;

    image(
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
    );

    void set_image(const raylib::Image& new_img);

    float rotation;

private:
    void render_element() const override;

    const image::display display_setting;
    const bool flip_horizontal;
    const bool flip_vertical;

    raylib::Texture texture;
};

enum class image::display
{
    shrink_to_fit,
    extend_to_fit
};

} // namespace ui
} // namespace kee