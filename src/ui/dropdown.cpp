#include "kee/ui/dropdown.hpp"

namespace kee {
namespace ui {

dropdown::dropdown(
    const kee::ui::base::required& reqs,
    kee::pos x,
    kee::pos y,
    const std::variant<kee::dims, kee::border>& dimensions,
    bool centered,
    std::vector<std::string>&& options,
    std::size_t start_idx
) :
    kee::ui::base(reqs, x, y, dimensions, centered),
    dropdown_outline_color(add_transition<kee::color>(kee::color::white())),
    dropdown_img_rotation(add_transition<float>(90.0f)),
    options_height(add_transition<float>(0.0f)),
    dropdown_rect(make_temp_child<kee::ui::rect>(
        raylib::Color::DarkGray(),
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        false,
        rect_outline(rect_outline::type::rel_w, 0.02f, raylib::Color::White()),
        std::nullopt
    )),
    dropdown_text_frame(make_temp_child<kee::ui::base>(
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.1f),
        true
    )),
    dropdown_text(dropdown_text_frame.make_temp_child<kee::ui::text>(
        raylib::Color::White(),
        pos(pos::type::rel, 0.05f),
        pos(pos::type::beg, 0),
        text_size(text_size::type::rel_h, 1),
        false, assets.font_regular, options[start_idx], false
    )),
    dropdown_button_frame(make_temp_child<kee::ui::base>(
        pos(pos::type::end, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    dropdown_button(dropdown_button_frame.make_temp_child<kee::ui::button>(
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_w, 0.3f),
        true
    )),
    dropdown_img(dropdown_button.make_temp_child<kee::ui::image>(
        assets.play_png,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0),
        true, false, false, 90.0f
    )),
    options_rect(make_temp_child<kee::ui::rect>(
        raylib::Color::DarkGray(),
        pos(pos::type::beg, 0),
        pos(pos::type::end, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, static_cast<float>(options.size() + 1))
        ),
        false, 
        rect_outline(rect_outline::type::rel_w, 0.02f, raylib::Color::White()),
        std::nullopt
    )),
    is_dropped_down(false),
    options(options)
{ 
    dropdown_button.on_event = [&](button::event button_event, [[maybe_unused]] bool ctrl_modifier)
    {
        switch (button_event)
        {
        case button::event::on_hot:
            this->dropdown_outline_color.set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
            break;
        case button::event::on_leave:
            this->dropdown_outline_color.set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    dropdown_button.on_click_l = [&]([[maybe_unused]] bool ctrl_modifier)
    {
        const float new_rotation = this->is_dropped_down ? 90.0f : -90.0f;

        this->dropdown_img_rotation.set(std::nullopt, new_rotation, 0.3f, kee::transition_type::exp);

        this->is_dropped_down = !this->is_dropped_down;
        if (this->is_dropped_down)
        {
            this->dropdown_img_rotation.set(std::nullopt, 90.0f, 0.3f, kee::transition_type::exp);
            this->options_height.set(std::nullopt, this->dropdown_rect.get_raw_rect().height * this->options.size(), 0.3f, kee::transition_type::exp);
            this->take_render_priority();
        }
        else
        {
            this->dropdown_img_rotation.set(std::nullopt, -90.0f, 0.3f, kee::transition_type::exp);
            this->options_height.set(std::nullopt, 0, 0.3f, kee::transition_type::exp);
            this->release_render_priority();
        }
    };
}

void dropdown::on_element_mouse_move(const raylib::Vector2& mouse_pos, bool ctrl_modifier)
{
    dropdown_button.on_element_mouse_move(mouse_pos, ctrl_modifier);
}

bool dropdown::on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, bool ctrl_modifier)
{
    return dropdown_button.on_element_mouse_down(mouse_pos, is_mouse_l, ctrl_modifier);
}

bool dropdown::on_element_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, bool ctrl_modifier)
{
    return dropdown_button.on_element_mouse_up(mouse_pos, is_mouse_l, ctrl_modifier);
}

void dropdown::update_element([[maybe_unused]] float dt)
{
    dropdown_rect.border.value().opt_color.value() = dropdown_outline_color.get().to_color();
    dropdown_img.set_opt_color(dropdown_outline_color.get().to_color());
    dropdown_img.rotation = dropdown_img_rotation.get();

    options_rect.y.val = -this->options_height.get();
}

void dropdown::render_element() const
{
    const raylib::Rectangle dropdown_raw_rect = dropdown_rect.get_raw_rect();

    /**
     * NOTE: `Begin/EndScissorMode` isn't available on `raylib-cpp`, so we must use the C API directly here.
     */
    BeginScissorMode(
        static_cast<int>(dropdown_raw_rect.x), 
        static_cast<int>(dropdown_raw_rect.y + dropdown_raw_rect.height), 
        static_cast<int>(dropdown_raw_rect.width), 
        static_cast<int>(options_height.get())
    );
    options_rect.render();
    EndScissorMode();

    dropdown_rect.render();
    dropdown_text.render();
    dropdown_img.render();
}


} // namespace ui
} // namespace kee