#include "kee/ui/dropdown.hpp"

namespace kee {
namespace ui {

dropdown::dropdown(
    const kee::ui::required& reqs,
    kee::pos x,
    kee::pos y,
    const std::variant<kee::dims, kee::border>& dimensions,
    bool centered,
    std::vector<std::string>&& options,
    std::size_t start_idx
) :
    kee::ui::base(reqs, x, y, dimensions, centered),
    on_select([]([[maybe_unused]] std::size_t idx){}),
    num_options(options.size()),
    dropdown_outline_color(add_transition<kee::color>(kee::color::white())),
    dropdown_img_rotation(add_transition<float>(90.0f)),
    options_height(add_transition<float>(0.0f)),
    options_curr_rect_y(add_transition<float>((start_idx + 1.0f) / (options.size() + 1.0f))),
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
    dropdown_button(make_temp_child<kee::ui::button>(
        pos(pos::type::beg, 0),
        pos(pos::type::beg, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1)
        ),
        false
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
    dropdown_img_frame(dropdown_button_frame.make_temp_child<kee::ui::base>(
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_w, 0.3f),
        true
    )),
    dropdown_img(dropdown_img_frame.make_temp_child<kee::ui::image>(
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
        false, std::nullopt, std::nullopt
    )),
    options_rect_border(options_rect.make_temp_child<kee::ui::rect>(
        raylib::Color::Blank(),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        border(border::type::rel_h, 0),
        false,
        rect_outline(rect_outline::type::rel_w, 0.02f, raylib::Color::White()),
        std::nullopt
    )),
    options_curr_rect(options_rect.make_temp_child<kee::ui::rect>(
        raylib::Color(60, 60, 60, 255),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, options_curr_rect_y.get()),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 1 / (options.size() + 1.0f))
        ),
        false, std::nullopt, std::nullopt
    )),
    is_dropped_down(false)
{
    options_buttons.reserve(options.size());
    options_button_text_frames.reserve(options.size());
    options_button_texts.reserve(options.size());

    for (std::size_t i = 0; i < options.size(); i++)
    {
        options_buttons.push_back(options_rect.make_temp_child<kee::ui::button>(
            pos(pos::type::rel, 0),
            pos(pos::type::rel, (i + 1.0f) / (options.size() + 1.0f)),
            dims(
                dim(dim::type::rel, 1),
                dim(dim::type::rel, 1 / (options.size() + 1.0f))
            ),
            false
        ));

        options_buttons[i].on_event = [&, idx = i](button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
        {
            switch (button_event)
            {
            case button::event::on_hot:
                this->options_button_texts[idx].set_opt_color(kee::color::dark_orange().to_color());
                break;
            case button::event::on_leave:
                this->options_button_texts[idx].set_opt_color(raylib::Color::White());
                break;
            default:
                break;
            }
        };

        options_buttons[i].on_click_l = [&, idx = i]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
        {
            this->options_curr_rect_y.set(std::nullopt, (idx + 1.0f) / (this->num_options + 1.0f), 0.2f, kee::transition_type::exp);
            this->dropdown_text.set_string(this->options_button_texts[idx].get_string());
            this->on_select(idx);
        };

        options_button_text_frames.push_back(options_buttons[i].make_temp_child<kee::ui::base>(
            pos(pos::type::rel, 0.5f),
            pos(pos::type::rel, 0.5f),
            border(border::type::rel_h, 0.1f),
            true
        ));

        options_button_texts.push_back(options_button_text_frames[i].make_temp_child<kee::ui::text>(
            raylib::Color::White(),
            pos(pos::type::rel, 0.05f),
            pos(pos::type::beg, 0),
            text_size(text_size::type::rel_h, 1),
            false, assets.font_regular, options[i], false
        ));
    }

    dropdown_button.on_event = [&](button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
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

    dropdown_button.on_click_l = [&]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        this->is_dropped_down = !this->is_dropped_down;
        if (this->is_dropped_down)
        {
            this->dropdown_img_rotation.set(std::nullopt, -90.0f, 0.3f, kee::transition_type::exp);
            this->options_height.set(std::nullopt, this->dropdown_rect.get_raw_rect().height * this->num_options, 0.3f, kee::transition_type::exp);
            this->take_render_priority();
        }
        else
        {
            this->dropdown_img_rotation.set(std::nullopt, 90.0f, 0.3f, kee::transition_type::exp);
            this->options_height.set(std::nullopt, 0, 0.3f, kee::transition_type::exp);
            this->release_render_priority();
        }
    };
}

void dropdown::on_element_mouse_move(const raylib::Vector2& mouse_pos, magic_enum::containers::bitset<kee::mods> mods)
{
    for (kee::ui::button& options_button : options_buttons)
        options_button.on_element_mouse_move(mouse_pos, mods);

    dropdown_button.on_element_mouse_move(mouse_pos, mods);
}

bool dropdown::on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods)
{
    if (is_dropped_down && !get_raw_rect().CheckCollision(mouse_pos) && !options_render_rect.CheckCollision(mouse_pos))
    {
        dropdown_button.on_click_l(magic_enum::containers::bitset<kee::mods>());
        return false;
    }

    if (options_render_rect.CheckCollision(mouse_pos))
        for (kee::ui::button& options_button : options_buttons)
            if (options_button.on_element_mouse_down(mouse_pos, is_mouse_l, mods))
                return true;

    return dropdown_button.on_element_mouse_down(mouse_pos, is_mouse_l, mods);
}

bool dropdown::on_element_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods)
{
    if (options_render_rect.CheckCollision(mouse_pos))
        for (kee::ui::button& options_button : options_buttons)
            if (options_button.on_element_mouse_up(mouse_pos, is_mouse_l, mods))
                return true;

    return dropdown_button.on_element_mouse_up(mouse_pos, is_mouse_l, mods);
}

void dropdown::update_element([[maybe_unused]] float dt)
{
    dropdown_rect.border.value().opt_color.value() = dropdown_outline_color.get().to_color();
    dropdown_img.set_opt_color(dropdown_outline_color.get().to_color());
    dropdown_img.rotation = dropdown_img_rotation.get();

    options_rect.y.val = -options_height.get();
    options_curr_rect.y.val = options_curr_rect_y.get();

    const raylib::Rectangle dropdown_raw_rect = dropdown_rect.get_raw_rect();
    options_render_rect = raylib::Rectangle(
        dropdown_raw_rect.x, 
        dropdown_raw_rect.y + dropdown_raw_rect.height, 
        dropdown_raw_rect.width, 
        options_height.get()
    );
}

void dropdown::render_element() const
{
    /**
     * NOTE: `Begin/EndScissorMode` isn't available on `raylib-cpp`, so we must use the C API directly here.
     */
    BeginScissorMode(
        static_cast<int>(options_render_rect.x),
        static_cast<int>(options_render_rect.y),
        static_cast<int>(options_render_rect.width),
        static_cast<int>(options_render_rect.height)
    );

    options_rect.render();
    options_curr_rect.render();
    for (const kee::ui::text& options_button_text : options_button_texts)
        options_button_text.render();
    options_rect_border.render();

    EndScissorMode();

    dropdown_rect.render();
    dropdown_text.render();
    dropdown_img.render();
}


} // namespace ui
} // namespace kee