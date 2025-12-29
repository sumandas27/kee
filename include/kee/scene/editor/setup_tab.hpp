#pragma once

#include "kee/ui/base.hpp"
#include "kee/ui/file_dialog.hpp"
#include "kee/ui/textbox.hpp"

namespace kee {
namespace scene {
namespace editor {

class key_color_state;
class video_state;
class root;
class setup_tab;

class setup_tab_info
{
public:
    setup_tab_info(
        const std::optional<beatmap_dir_info>& dir_info, 
        const kee::image_texture& exit_png, 
        std::optional<video_state>& vid_state,
        key_color_state& key_colors
    );

    const kee::image_texture& exit_png;

    std::optional<video_state>& vid_state;
    key_color_state& key_colors;

    std::optional<std::filesystem::path> new_song_path; /* TODO: change pattern to match other assets */
    std::optional<std::filesystem::path> img_path; /* TODO: prob best to merge path with image texture imo, same w/ vid + key color state */

    bool from_dir;
    std::string song_artist;
    std::string song_name;
    std::string mapper;
    std::string level_name;

    float beat_forgiveness;
};

class video_offset_ui
{
public:
    video_offset_ui(kee::ui::base& r_side_frame, const raylib::Font& font_semi_bold, setup_tab& setup_tab_elem, float video_offset);

    setup_tab& setup_tab_elem;

    kee::ui::handle<kee::ui::text> video_offset_label;
    kee::ui::handle<kee::ui::base> video_offset_frame;
    kee::ui::handle<kee::ui::textbox> video_offset_textbox;
};

class setup_tab : public kee::ui::base
{
public:
    setup_tab(const kee::ui::required& reqs, root& root_elem, setup_tab_info& setup_info, float& approach_beats);

private:
    static const std::string_view no_key_colors_message;
    static const std::string_view no_img_message;
    static const std::string_view no_vid_message;

    void init_video_offset_ui();

    void update_element(float dt) override;

    root& root_elem;
    setup_tab_info& setup_info;
    float& approach_beats;

    kee::transition<kee::color>& key_color_remove_button_color;
    kee::transition<kee::color>& image_remove_button_color;
    kee::transition<kee::color>& video_remove_button_color;

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
    kee::ui::handle<kee::ui::button> key_color_remove_button;
    kee::ui::handle<kee::ui::image> key_color_remove_image;

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
    std::optional<video_offset_ui> video_offset;
};

} // namespace editor
} // namespace scene
} // namespace kee