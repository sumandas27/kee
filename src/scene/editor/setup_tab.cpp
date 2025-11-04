#include "kee/scene/editor/setup_tab.hpp"

#include <charconv>

#include "kee/scene/editor/root.hpp"

namespace kee {
namespace scene {
namespace editor {

setup_tab::setup_tab(const kee::ui::required& reqs, kee::scene::editor::root& editor_scene) :
    kee::ui::base(reqs,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.04f),
        dims(
            dim(dim::type::rel, 1.0f),
            dim(dim::type::rel, 0.88f)
        ),
        false
    ),
    editor_scene(editor_scene),
    metadata_bg(add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(20, 20, 20),
        pos(pos::type::rel, 4.0f / 30.0f),
        pos(pos::type::rel, 0.05f),
        dims(
            dim(dim::type::rel, 0.3f),
            dim(dim::type::rel, 0.9f)
        ),
        false, std::nullopt,
        ui::rect_roundness(ui::rect_roundness::type::rel_w, 0.1f, std::nullopt)
    )),
    metadata_label(metadata_bg.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.05f),
        ui::text_size(ui::text_size::type::rel_h, 0.05f),
        true, assets.font_semi_bold, "METADATA", false
    )),
    audio_text(metadata_bg.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.12f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "AUDIO", false
    )),
    audio_file_dialog(metadata_bg.ref.add_child<kee::ui::file_dialog>(std::nullopt,
        pos(pos::type::rel, 0.45f),
        pos(pos::type::rel, 0.12f),
        dims(
            dim(dim::type::rel, 0.45f),
            dim(dim::type::rel, 0.03f)
        ),
        false,
        std::vector<kee::ui::file_dialog_filter>({
            kee::ui::file_dialog_filter("MP3 File", ".mp3")
        })
    )),
    artist_text(metadata_bg.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.1825f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "ARTIST", false
    )),
    artist_textbox(metadata_bg.ref.add_child<kee::ui::textbox>(std::nullopt,
        pos(pos::type::rel, 0.45f),
        pos(pos::type::rel, 0.1825f),
        dims(
            dim(dim::type::rel, 0.45f),
            dim(dim::type::rel, 0.03f)
        ),
        false, *this
    )),
    song_name_text(metadata_bg.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.245f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "SONG NAME", false
    )),
    song_name_textbox(metadata_bg.ref.add_child<kee::ui::textbox>(std::nullopt,
        pos(pos::type::rel, 0.45f),
        pos(pos::type::rel, 0.245f),
        dims(
            dim(dim::type::rel, 0.45f),
            dim(dim::type::rel, 0.03f)
        ),
        false, *this
    )),
    difficulty_bg(add_child<kee::ui::rect>(std::nullopt,
        raylib::Color(20, 20, 20),
        pos(pos::type::rel, 17.0f / 30.0f),
        pos(pos::type::rel, 0.05f),
        dims(
            dim(dim::type::rel, 0.3f),
            dim(dim::type::rel, 0.9f)
        ),
        false, std::nullopt,
        ui::rect_roundness(ui::rect_roundness::type::rel_w, 0.1f, std::nullopt)
    )),
    difficulty_label(difficulty_bg.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.05f),
        ui::text_size(ui::text_size::type::rel_h, 0.05f),
        true, assets.font_semi_bold, "DIFFICULTY", false
    )),
    approach_text(difficulty_bg.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.12f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "APPROACH BEATS", false
    )),
    approach_textbox(difficulty_bg.ref.add_child<kee::ui::textbox>(std::nullopt,
        pos(pos::type::rel, 0.65f),
        pos(pos::type::rel, 0.12f),
        dims(
            dim(dim::type::rel, 0.25f),
            dim(dim::type::rel, 0.03f)
        ),
        false, *this
    )),
    forgiveness_text(difficulty_bg.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.1825f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "BEAT FORGIVENESS", false
    )),
    forgiveness_textbox(difficulty_bg.ref.add_child<kee::ui::textbox>(std::nullopt,
        pos(pos::type::rel, 0.65f),
        pos(pos::type::rel, 0.1825f),
        dims(
            dim(dim::type::rel, 0.25f),
            dim(dim::type::rel, 0.03f)
        ),
        false, *this
    ))
{ 
    audio_file_dialog.ref.on_filter_mismatch = [&]()
    {
        this->editor_scene.set_error("Not a valid audio file format!", true);
    };

    approach_textbox.ref.on_string_input = [&](std::string_view new_str) -> bool
    {
        float text_float;

        auto [ptr, ec] = std::from_chars(new_str.data(), new_str.data() + new_str.size(), text_float);
        if (ec != std::errc() || ptr != new_str.data() + new_str.size())
            return false;

        /* TODO: complete */
        return true;
    };

    forgiveness_textbox.ref.on_string_input = [&](std::string_view new_str) -> bool
    {
        float text_float;

        auto [ptr, ec] = std::from_chars(new_str.data(), new_str.data() + new_str.size(), text_float);
        if (ec != std::errc() || ptr != new_str.data() + new_str.size())
            return false;

        /* TODO: complete */
        return true;
    };
}

} // namespace editor
} // namespace scene
} // namespace kee