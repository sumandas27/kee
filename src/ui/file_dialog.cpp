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
    on_success([]([[maybe_unused]] std::filesystem::path selected_file){}),
    on_filter_mismatch([](){}),
    fd_outline_color(add_transition<kee::color>(kee::color::white())),
    fd_rect(make_temp_child<kee::ui::rect>(
        raylib::Color(50, 50, 50),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        false,
        rect_outline(rect_outline::type::rel_h, 0.07f, fd_outline_color.get().to_color()),
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
    fd_button(make_temp_child<kee::ui::button>(
        pos(pos::type::end, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    fd_image(fd_button.make_temp_child<kee::ui::image>(
        assets.directory_png,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_w, 0.2f),
        true, false, false, 0.0f
    ))
{ 
    fd_button.on_event = [&](button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case button::event::on_hot:
            this->fd_outline_color.set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
            break;
        case button::event::on_leave:
            this->fd_outline_color.set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    fd_button.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        /* TODO: code filter as file_dialog param */
        nfdfilteritem_t mp3_filter;
        mp3_filter.name = "MP3 File";
        mp3_filter.spec = "mp3";

        NFD::UniquePath out_path;
        nfdresult_t open_dialog_res = NFD::OpenDialog(out_path, &mp3_filter, 1);
        switch (open_dialog_res)
        {
        case NFD_OKAY:
            /* TODO: check if file is actually mp3 */
            /* TODO: call func lambda */
            this->on_filter_mismatch();
            break;
        case NFD_CANCEL:
            break;
        case NFD_ERROR:
            const std::string nfd_error_str = std::format("kee::ui::file_dialog on_click_l - NFD Error: {}", NFD::GetError());
            throw std::runtime_error(nfd_error_str);
        }
    };
}

void file_dialog::on_element_mouse_move(const raylib::Vector2& mouse_pos, magic_enum::containers::bitset<kee::mods> mods)
{
    fd_button.on_element_mouse_move(mouse_pos, mods);
}

bool file_dialog::on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods)
{
    return fd_button.on_element_mouse_down(mouse_pos, is_mouse_l, mods);
}

bool file_dialog::on_element_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods)
{
    return fd_button.on_element_mouse_up(mouse_pos, is_mouse_l, mods);
}

void file_dialog::update_element([[maybe_unused]] float dt)
{
    fd_rect.border.value().opt_color = fd_outline_color.get().to_color();
    fd_image.set_opt_color(fd_outline_color.get().to_color());
}

void file_dialog::render_element() const
{
    fd_rect.render();
    fd_text.render();
    fd_image.render();
}

} // namespace ui
} // namespace kee