#pragma once

#include "kee/ui/base.hpp"
#include "kee/ui/file_dialog.hpp"
#include "kee/ui/textbox.hpp"

namespace kee {
namespace scene {
namespace editor {

class root;

class setup_tab : public kee::ui::base
{
public:
    setup_tab(const kee::ui::required& reqs, kee::scene::editor::root& editor_scene);

private:
    kee::scene::editor::root& editor_scene;

    kee::ui::handle<kee::ui::rect> metadata_bg;
    kee::ui::handle<kee::ui::text> metadata_label;

    kee::ui::handle<kee::ui::text> audio_text;
    kee::ui::handle<kee::ui::file_dialog> audio_file_dialog;

    kee::ui::handle<kee::ui::text> artist_text;
    kee::ui::handle<kee::ui::textbox> artist_textbox;
    
    kee::ui::handle<kee::ui::text> song_name_text;
    kee::ui::handle<kee::ui::textbox> song_name_textbox;

    kee::ui::handle<kee::ui::rect> difficulty_bg;
    kee::ui::handle<kee::ui::text> difficulty_label;
};

} // namespace editor
} // namespace scene
} // namespace kee