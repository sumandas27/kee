#include "kee/scene/editor/metadata_tab.hpp"

#include "kee/scene/editor/root.hpp"

namespace kee {
namespace scene {
namespace editor {

metadata_tab::metadata_tab(const kee::ui::required& reqs, kee::scene::editor::root& editor_scene) :
    kee::ui::base(reqs,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.04f),
        dims(
            dim(dim::type::rel, 1.0f),
            dim(dim::type::rel, 0.96f)
        ),
        false
    ),
    editor_scene(editor_scene),
    audio_text(add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.2f),
        pos(pos::type::rel, 0.05f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "AUDIO", false
    )),
    audio_file_dialog(add_child<kee::ui::file_dialog>(std::nullopt,
        pos(pos::type::rel, 0.35f),
        pos(pos::type::rel, 0.05f),
        dims(
            dim(dim::type::rel, 0.45f),
            dim(dim::type::rel, 0.03f)
        ),
        false
    )),
    artist_text(add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.2f),
        pos(pos::type::rel, 0.1125f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "ARTIST", false
    )),
    song_name_text(add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.2f),
        pos(pos::type::rel, 0.175f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "SONG NAME", false
    ))
{ 
    audio_file_dialog.ref.on_filter_mismatch = [&]()
    {
        this->editor_scene.set_error("Not a valid audio file format!", true);
    };
}

} // namespace editor
} // namespace scene
} // namespace kee