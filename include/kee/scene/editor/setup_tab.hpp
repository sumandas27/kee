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
    setup_tab_info(const std::optional<beatmap_dir_info>& dir_info, const kee::image_texture& exit_png);

    const kee::image_texture& exit_png;

    std::optional<std::filesystem::path> new_song_path;
    std::optional<std::filesystem::path> img_path;

    bool from_dir;
    std::string song_artist;
    std::string song_name;
    std::string mapper;
    std::string level_name;

    float beat_forgiveness;
};

class setup_tab : public kee::ui::base
{
public:
    setup_tab(const kee::ui::required& reqs, root& root_elem, setup_tab_info& setup_info, float& approach_beats);

private:
    static const std::string_view no_img_message;
    static const std::string_view no_vid_message;

    void update_element(float dt) override;

    root& root_elem;
    setup_tab_info& setup_info;
    float& approach_beats;

    kee::transition<kee::color>& image_remove_button_color;
    /* TODO: add video_remove_button_color and all its coding */

    kee::ui::handle<kee::ui::base> l_side_frame;
    kee::ui::handle<kee::ui::rect> audio_bg;
    kee::ui::handle<kee::ui::text> audio_label;

    kee::ui::handle<kee::ui::text> audio_text;
    kee::ui::handle<kee::ui::file_dialog> audio_file_dialog;

    kee::ui::handle<kee::ui::text> artist_text;
    kee::ui::handle<kee::ui::textbox> artist_textbox;
    
    kee::ui::handle<kee::ui::text> song_name_text;
    kee::ui::handle<kee::ui::textbox> song_name_textbox;

    kee::ui::handle<kee::ui::rect> metadata_bg;
    kee::ui::handle<kee::ui::text> metadata_label;

    kee::ui::handle<kee::ui::text> mapper_text;
    kee::ui::handle<kee::ui::textbox> mapper_textbox;

    kee::ui::handle<kee::ui::text> level_name_text;
    kee::ui::handle<kee::ui::textbox> level_name_textbox;

    kee::ui::handle<kee::ui::base> r_side_frame;
    kee::ui::handle<kee::ui::rect> difficulty_bg;
    kee::ui::handle<kee::ui::text> difficulty_label;

    kee::ui::handle<kee::ui::text> approach_text;
    kee::ui::handle<kee::ui::textbox> approach_textbox;

    kee::ui::handle<kee::ui::text> forgiveness_text;
    kee::ui::handle<kee::ui::textbox> forgiveness_textbox;

    kee::ui::handle<kee::ui::rect> decoration_bg;
    kee::ui::handle<kee::ui::text> decoration_label;

    kee::ui::handle<kee::ui::text> key_color_text;
    kee::ui::handle<kee::ui::base> key_color_frame;
    kee::ui::handle<kee::ui::file_dialog> key_color_dialog;

    kee::ui::handle<kee::ui::text> image_text;
    kee::ui::handle<kee::ui::base> image_frame;
    kee::ui::handle<kee::ui::file_dialog> image_dialog;
    kee::ui::handle<kee::ui::button> image_remove_button;
    kee::ui::handle<kee::ui::image> image_remove_img;

    kee::ui::handle<kee::ui::text> video_text;
    kee::ui::handle<kee::ui::base> video_frame;
    kee::ui::handle<kee::ui::file_dialog> video_dialog;
    kee::ui::handle<kee::ui::button> video_remove_button;
    kee::ui::handle<kee::ui::image> video_remove_img;
};

} // namespace editor
} // namespace scene
} // namespace kee