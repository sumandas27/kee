#pragma once

#include "kee/ui/base.hpp"
#include "kee/ui/button.hpp"
#include "kee/ui/image.hpp"
#include "kee/ui/rect.hpp"
#include "kee/ui/text.hpp"

namespace kee {
namespace ui {

class file_dialog : public kee::ui::base
{
public:
    file_dialog(
        const kee::ui::required& reqs,
        kee::pos x,
        kee::pos y,
        const std::variant<kee::dims, kee::border>& dimensions,
        bool centered
    );

private:
    void render_element() const override;

    kee::ui::rect fd_rect;
    //kee::ui::button button;
    
    kee::ui::base fd_text_frame;
    kee::ui::text fd_text;

    kee::ui::base fd_image_frame;
    kee::ui::image fd_image;
};

} // namespace ui
} // namespace kee