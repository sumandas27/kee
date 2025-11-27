#include "kee/ui/file_dialog.hpp"

namespace kee {
namespace ui {

file_dialog_filter::file_dialog_filter(std::string_view name, std::string_view spec) :
    name(name),
    spec(spec)
{ }

file_dialog::file_dialog(
    const kee::ui::required& reqs,
    const kee::pos& x,
    const kee::pos& y,
    const std::variant<kee::dims, kee::border>& dimensions,
    bool centered,
    std::vector<file_dialog_filter> filters,
    std::variant<std::string_view, std::filesystem::path> initial_msg
) :
    kee::ui::base(reqs, x, y, dimensions, centered),
    on_success([]([[maybe_unused]] std::filesystem::path selected_file){}),
    on_filter_mismatch([](){}),
    fd_outline_color(add_transition<kee::color>(kee::color::white)),
    fd_rect(make_temp_child<kee::ui::rect>(
        kee::color(50, 50, 50),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        false,
        rect_outline(rect_outline::type::rel_h, 0.07f, fd_outline_color.get()),
        rect_roundness(rect_roundness::type::rel_h, 0.15f, std::nullopt)
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
        assets.directory_png, kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_w, 0.2f),
        true, false, false, 0.0f
    )),
    fd_text_area(make_temp_child<kee::ui::base>(
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::abs, 0),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    fd_text_frame(fd_text_area.make_temp_child<kee::ui::base>(
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.15f),
        true
    )),
    fd_text(fd_text_frame.make_temp_child<kee::ui::text>(
        kee::color::white,
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        text_size(text_size::type::rel_h, 1),
        false, assets.font_regular, std::holds_alternative<std::string_view>(initial_msg)
            ? std::get<std::string_view>(initial_msg)
            : std::get<std::filesystem::path>(initial_msg).filename().string(), 
        false
    )),
    filters(filters),
    old_path(std::holds_alternative<std::filesystem::path>(initial_msg)
        ? std::make_optional(std::get<std::filesystem::path>(initial_msg))
        : std::nullopt
    )
{
    const raylib::Rectangle fd_raw_rect = get_raw_rect();
    std::get<kee::dims>(fd_text_area.dimensions).w.val = fd_raw_rect.width - fd_raw_rect.height;

    fd_button.on_event = [&](button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case button::event::on_hot:
            this->fd_outline_color.set(std::nullopt, kee::color::dark_orange, 0.5f, kee::transition_type::exp);
            break;
        case button::event::on_leave:
            this->fd_outline_color.set(std::nullopt, kee::color::white, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    fd_button.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        NFD::UniquePath out_path;
        nfdresult_t open_dialog_res = NFD::OpenDialog(out_path);

        switch (open_dialog_res)
        {
        case NFD_OKAY: {
            const std::filesystem::path selected_path(out_path.get());
            if (old_path.has_value() && std::filesystem::equivalent(old_path.value(), selected_path))
                return;

            const bool filter_match = std::any_of(this->filters.begin(), this->filters.end(), 
                [&](const file_dialog_filter& fd_filter)
                {
                    return selected_path.extension() == fd_filter.spec;
                }
            );

            if (filter_match)
            {
                this->on_success(std::filesystem::absolute(selected_path).string());
                this->fd_text.set_string(selected_path.filename().string());
                this->old_path = selected_path;
            }
            else
                this->on_filter_mismatch();

            return;
        }
        case NFD_CANCEL:
            return;
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
    fd_rect.border.value().color = fd_outline_color.get();
    fd_image.color = fd_outline_color.get();
}

void file_dialog::render_element() const
{
    fd_rect.render();
    fd_image.render();

    /**
     * `raylib-cpp` does not support `Begin/EndScissorMode`.
     */
    const raylib::Rectangle text_frame_rect = fd_text_frame.get_raw_rect();
    BeginScissorMode(
        static_cast<int>(text_frame_rect.x),
        static_cast<int>(text_frame_rect.y),
        static_cast<int>(text_frame_rect.width),
        static_cast<int>(text_frame_rect.height)
    );

    fd_text.render();

    EndScissorMode();
}

} // namespace ui
} // namespace kee