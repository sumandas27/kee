#include "kee/scene/editor/root.hpp"

namespace kee {
namespace scene {
namespace editor {

root::root(const kee::scene::window& window, kee::game& game, kee::global_assets& assets) :
    kee::scene::base(window, game, assets),
    exit_png("assets/img/exit.png"),
    error_png("assets/img/error.png"),
    active_tab_elem(add_child<metadata_tab>(std::nullopt, *this)),
    active_tab(root::tabs::metadata),
    tab_active_rect_rel_x(add_transition<float>(static_cast<float>(active_tab) / magic_enum::enum_count<root::tabs>())),
    exit_button_rect_alpha(add_transition<float>(0.0f)),
    error_rect_rel_x(add_transition<float>(1.0f)),
    error_alpha(add_transition<float>(0)),
    tab_rect(add_child<kee::ui::rect>(1,
        raylib::Color(10, 10, 10, 255),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.04f)
        ),
        false, std::nullopt, std::nullopt
    )),
    tab_display_frame(tab_rect.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::abs, 0),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    tab_active_rect(tab_display_frame.ref.add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(20, 20, 20, 255),
        pos(pos::type::rel, tab_active_rect_rel_x.get()),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 1.0f / magic_enum::enum_count<root::tabs>()),
            dim(dim::type::rel, 1)
        ),
        false, std::nullopt, std::nullopt
    )),
    exit_button(tab_rect.ref.add_child<kee::ui::button>(std::nullopt,
        pos(pos::type::end, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    exit_button_rect(exit_button.ref.add_child<kee::ui::rect>(0,
        raylib::Color(255, 0, 0, static_cast<unsigned char>(exit_button_rect_alpha.get())),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::abs, 0),
        true, std::nullopt, std::nullopt
    )),
    exit_button_image(exit_button.ref.add_child<kee::ui::image>(1,
        exit_png,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_w, 0.3f),
        true, false, false, 0.0f
    )),
    error_rect(add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(80, 80, 80, static_cast<unsigned char>(error_alpha.get())),
        pos(pos::type::rel, error_rect_rel_x.get()),
        pos(pos::type::rel, 0.93f),
        dims(
            dim(dim::type::rel, 0.2f),
            dim(dim::type::rel, 0.05f)
        ),
        false,
        ui::rect_outline(ui::rect_outline::type::rel_h, 0.1f, raylib::Color(255, 0, 0, static_cast<unsigned char>(error_alpha.get()))),
        ui::rect_roundness(ui::rect_roundness::type::rel_h, 0.2f, std::nullopt)
    )),
    error_img_frame(error_rect.ref.add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::aspect, 1),
            dim(dim::type::rel, 1)
        ),
        false
    )),
    error_img(error_img_frame.ref.add_child<kee::ui::image>(std::nullopt,
        error_png,
        raylib::Color(255, 0, 0, static_cast<unsigned char>(error_alpha.get())),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.5f),
        border(border::type::rel_h, 0.3f),
        true, false, false, 0.0f
    )),
    error_text(error_rect.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color(255, 255, 255, static_cast<unsigned char>(error_alpha.get())),
        pos(pos::type::rel, 0.13f),
        pos(pos::type::rel, 0.3f),
        ui::text_size(ui::text_size::type::rel_h, 0.4f),
        false, assets.font_semi_bold, std::string(), false
    )),
    error_timer(0.0f)
{
    std::visit([](const auto& elem) {
        elem.ref.take_keyboard_capture();
    }, active_tab_elem);

    const raylib::Rectangle tab_raw_rect = tab_rect.ref.get_raw_rect();
    std::get<kee::dims>(tab_display_frame.ref.dimensions).w.val = tab_raw_rect.width - tab_raw_rect.height;

    exit_button.ref.on_event = [&](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
    {
        switch (button_event)
        {
        case ui::button::event::on_hot:
            this->exit_button_rect_alpha.set(std::nullopt, 255, 0.5f, kee::transition_type::exp);
            break;
        case ui::button::event::on_leave:
            this->exit_button_rect_alpha.set(std::nullopt, 0, 0.5f, kee::transition_type::exp);
            break;
        default:
            break;
        }
    };

    tab_buttons.reserve(magic_enum::enum_count<root::tabs>());
    tab_button_text.reserve(magic_enum::enum_count<root::tabs>());

    for (std::size_t i = 0; i < magic_enum::enum_count<root::tabs>(); i++)
    {
        tab_button_text_colors.push_back(add_transition<kee::color>(kee::color::white()));

        tab_buttons.push_back(tab_display_frame.ref.add_child<kee::ui::button>(std::nullopt,
            pos(pos::type::rel, static_cast<float>(i) / magic_enum::enum_count<root::tabs>()),
            pos(pos::type::rel, 0),
            dims(
                dim(dim::type::rel, 1.0f / magic_enum::enum_count<root::tabs>()),
                dim(dim::type::rel, 1)
            ),
            false
        ));

        tab_buttons.back().ref.on_event = [&, idx = i](ui::button::event button_event, [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
        {
            switch (button_event)
            {
            case ui::button::event::on_hot:
                this->tab_button_text_colors[idx].get().set(std::nullopt, kee::color::dark_orange(), 0.5f, kee::transition_type::exp);
                break;
            case ui::button::event::on_leave:
                this->tab_button_text_colors[idx].get().set(std::nullopt, kee::color::white(), 0.5f, kee::transition_type::exp);
                break;
            default:
                break;
            }
        };

        tab_buttons.back().ref.on_click_l = [&, idx = i]([[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods)
        {
            const float new_rel_x = static_cast<float>(idx) / magic_enum::enum_count<root::tabs>();
            this->tab_active_rect_rel_x.set(std::nullopt, new_rel_x, 0.3f, kee::transition_type::exp);

            const root::tabs tab_enum = static_cast<root::tabs>(idx);
            if (this->active_tab == tab_enum)
                return;

            std::visit([](const auto& elem) {
                elem.ref.release_keyboard_capture();
            }, this->active_tab_elem);

            this->active_tab = tab_enum;
            switch (this->active_tab)
            {
            case root::tabs::metadata:
                this->active_tab_elem.emplace<kee::ui::handle<metadata_tab>>(add_child<metadata_tab>(std::nullopt, *this));
                break;
            case root::tabs::compose:
                this->active_tab_elem.emplace<kee::ui::handle<compose_tab>>(add_child<compose_tab>(std::nullopt, compose_info));
                break;
            case root::tabs::decoration:
                this->active_tab_elem.emplace<kee::ui::handle<decoration_tab>>(add_child<decoration_tab>(std::nullopt));
                break;
            case root::tabs::timing:
                this->active_tab_elem.emplace<kee::ui::handle<timing_tab>>(add_child<timing_tab>(std::nullopt));
                break;
            }

            std::visit([](const auto& elem) {
                elem.ref.take_keyboard_capture();
            }, this->active_tab_elem);
        };

        const root::tabs tab_enum = static_cast<root::tabs>(i);
        std::string enum_name = std::string(magic_enum::enum_name(tab_enum));
        std::transform(enum_name.begin(), enum_name.end(), enum_name.begin(), [](unsigned char c) { return static_cast<unsigned char>(std::toupper(c)); });

        tab_button_text.push_back(tab_buttons.back().ref.add_child<kee::ui::text>(std::nullopt,
            raylib::Color::White(),
            pos(pos::type::rel, 0.5f),
            pos(pos::type::rel, 0.5f),
            ui::text_size(ui::text_size::type::rel_h, 0.6f),
            true, assets.font_semi_bold, enum_name, false
        ));
    }
}

void root::set_error(std::string_view error_str, bool from_file_dialog)
{
    error_text.ref.set_string(error_str);
    /**
     * File dialogs hang the program during file selection, making that frame's 
     * `dt` significantly large, messing up transition timings for the error's UI.
     * These frames are skipped before error UI rendering is triggered.
     */
    error_skips_before_start = from_file_dialog ? 2 : 0;
}

void root::update_element(float dt)
{
    /* TODO: fix this */
    if (error_timer > 0.0f)
    {
        error_timer -= dt;
        if (error_timer <= 0.0f)
        {
            error_alpha.set(255.0f, 0.0f, root::error_transition_time, kee::transition_type::exp);
            error_rect_rel_x.set(0.79f, 1.0f, root::error_transition_time, kee::transition_type::exp);

            error_timer = 0.0f;
        }
    }

    if (error_skips_before_start.has_value())
    {
        if (error_skips_before_start.value() != 0)
            error_skips_before_start.value()--;
        else
        {
            error_alpha.set(0.0f, 255.0f, root::error_transition_time, kee::transition_type::exp);
            error_rect_rel_x.set(1.0f, 0.79f, root::error_transition_time, kee::transition_type::exp);

            error_timer = 3.0f + root::error_transition_time;
            error_skips_before_start.reset();
        }
    }

    for (std::size_t i = 0; i < tab_button_text.size(); i++)
        tab_button_text[i].ref.set_opt_color(tab_button_text_colors[i].get().get().to_color());

    tab_active_rect.ref.x.val = tab_active_rect_rel_x.get();
    exit_button_rect.ref.set_opt_color(raylib::Color(255, 0, 0, static_cast<unsigned char>(exit_button_rect_alpha.get())));

    const unsigned char error_a = static_cast<unsigned char>(error_alpha.get());
    error_rect.ref.set_opt_color(raylib::Color(80, 80, 80, error_a));
    error_rect.ref.border.value().opt_color = raylib::Color(255, 0, 0, error_a);
    error_img.ref.set_opt_color(raylib::Color(255, 0, 0, error_a));
    error_text.ref.set_opt_color(raylib::Color(255, 255, 255, error_a));

    error_rect.ref.x.val = error_rect_rel_x.get();
}

} // namespace editor
} // namespace scene
} // namespace kee