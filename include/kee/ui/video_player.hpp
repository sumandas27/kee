#pragma once

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

private:
    void render_element() const override;

    raylib::Texture curr_texture;
};

} // namespace ui
} // namespace kee