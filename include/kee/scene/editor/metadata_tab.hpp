#pragma once

#include "kee/ui/base.hpp"
#include "kee/ui/file_dialog.hpp"

namespace kee {
namespace scene {
namespace editor {

class metadata_tab : public kee::ui::base
{
public:
    metadata_tab(const kee::ui::required& reqs);

private:
    kee::ui::handle<kee::ui::text> audio_text;
    kee::ui::handle<kee::ui::file_dialog> audio_file_dialog;

    kee::ui::handle<kee::ui::text> artist_text;
    /* TODO: textbox */
    
    kee::ui::handle<kee::ui::text> song_name_text;
    /* TODO: textbox */
};

} // namespace editor
} // namespace scene
} // namespace kee