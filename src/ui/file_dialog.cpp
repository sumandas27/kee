#include "kee/ui/file_dialog.hpp"

namespace kee {
namespace ui {

file_dialog::file_dialog(
    const kee::ui::required& reqs,
    kee::pos x,
    kee::pos y,
    const std::variant<kee::dims, kee::border>& dimensions,
    bool centered
) :
    kee::ui::base(reqs, x, y, dimensions, centered),
    fd_rect(make_temp_child<kee::ui::rect>(
        raylib::Color(50, 50, 50),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        false,
        rect_outline(rect_outline::type::rel_h, 0.07f, raylib::Color::White()),
        std::nullopt
    )),
    fd_text_frame(make_temp_child<kee::ui::base>(
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.1f),
        true
    )),
    fd_text(fd_text_frame.make_temp_child<kee::ui::text>(
        raylib::Color::White(),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        text_size(text_size::type::rel_h, 1),
        false, assets.font_regular, "No file selected", false
    )),
    fd_image_frame(make_temp_child<kee::ui::base>(
        pos(pos::type::end, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    fd_image(fd_image_frame.make_temp_child<kee::ui::image>(
        assets.play_png,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_w, 0.3f),
        true, false, false, 0.0f
    ))
{ }

void file_dialog::render_element() const
{
    fd_rect.render();
    fd_text.render();
    fd_image.render();
}

} // namespace ui
} // namespace kee