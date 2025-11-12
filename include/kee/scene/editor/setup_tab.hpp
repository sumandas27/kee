#pragma once

#include "kee/ui/base.hpp"
#include "kee/ui/file_dialog.hpp"
#include "kee/ui/textbox.hpp"

namespace kee {
namespace scene {
namespace editor {

class root;

class setup_tab_info
{
public:
    setup_tab_info();

    std::string get_song_fd_init_msg() const;

    /**
     * bool                     -   Is this a newly created beatmap?
     * std::filesystem::path    -   Any previously selected song from the file dialog.
     */
    std::variant<bool, std::filesystem::path> song_path_info;
    bool save_song_needed;

    std::string song_artist;
    std::string song_name;

    float beat_forgiveness;
};

class setup_tab : public kee::ui::base
{
public:
    setup_tab(const kee::ui::required& reqs, root& root_elem, setup_tab_info& setup_info, float& approach_beats);

private:
    root& root_elem;
    setup_tab_info& setup_info;
    float& approach_beats;

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

    kee::ui::handle<kee::ui::text> approach_text;
    kee::ui::handle<kee::ui::textbox> approach_textbox;

    kee::ui::handle<kee::ui::text> forgiveness_text;
    kee::ui::handle<kee::ui::textbox> forgiveness_textbox;
};

} // namespace editor
} // namespace scene
} // namespace kee