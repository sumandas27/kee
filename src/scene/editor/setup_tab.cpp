#include "kee/scene/editor/setup_tab.hpp"

#include "kee/scene/editor/root.hpp"

namespace kee {
namespace scene {
namespace editor {

setup_tab_info::setup_tab_info(const std::optional<beatmap_dir_info>& dir_info) :
    from_dir(dir_info.has_value()),
    song_artist(dir_info.has_value() ? dir_info.value().song_artist : ""),
    song_name(dir_info.has_value() ? dir_info.value().song_name : ""),
    mapper(dir_info.has_value() ? dir_info.value().mapper : ""),
    level_name(dir_info.has_value() ? dir_info.value().level_name : ""),
    beat_forgiveness(dir_info.has_value() ? dir_info.value().beat_forgiveness : 0.25f)
{ }

setup_tab::setup_tab(const kee::ui::required& reqs, root& root_elem, setup_tab_info& setup_info, float& approach_beats) :
    kee::ui::base(reqs,
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.04f),
        dims(
            dim(dim::type::rel, 1.0f),
            dim(dim::type::rel, 0.88f)
        ),
        false
    ),
    root_elem(root_elem),
    setup_info(setup_info),
    approach_beats(approach_beats),
    l_side_frame(add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.05f),
        dims(
            dim(dim::type::rel, 0.35f),
            dim(dim::type::rel, 0.9f)
        ),
        false
    )),
    audio_bg(l_side_frame.ref.add_child<kee::ui::rect>(0,
        kee::color(20, 20, 20),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.4f)
        ),
        false, std::nullopt,
        ui::rect_roundness(ui::rect_roundness::type::rel_w, 0.1f, std::nullopt)
    )),
    audio_label(l_side_frame.ref.add_child<kee::ui::text>(1,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.05f),
        ui::text_size(ui::text_size::type::rel_h, 0.05f),
        true, assets.font_semi_bold, "AUDIO", true
    )),
    audio_text(l_side_frame.ref.add_child<kee::ui::text>(1,
        kee::color::white,
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.12f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "FILE", false
    )),
    audio_file_dialog(l_side_frame.ref.add_child<kee::ui::file_dialog>(1,
        pos(pos::type::rel, 0.45f),
        pos(pos::type::rel, 0.12f),
        dims(
            dim(dim::type::rel, 0.45f),
            dim(dim::type::rel, 0.03f)
        ),
        false,
        std::vector<std::string_view>({".mp3"}),
        [&]() -> std::variant<std::string_view, std::filesystem::path>
        {
            if (setup_info.new_song_path.has_value())
                return setup_info.new_song_path.value().filename();
            else if (setup_info.from_dir)
                return root_elem.save_info.value().file_dir / "song.mp3";
            else
                return std::string_view("Select a song");
        }()
    )),
    artist_text(l_side_frame.ref.add_child<kee::ui::text>(1,
        kee::color::white,
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.1825f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "ARTIST", false
    )),
    artist_textbox(l_side_frame.ref.add_child<kee::ui::textbox>(1,
        pos(pos::type::rel, 0.45f),
        pos(pos::type::rel, 0.1825f),
        dims(
            dim(dim::type::rel, 0.45f),
            dim(dim::type::rel, 0.03f)
        ),
        false, *this, setup_info.song_artist
    )),
    song_name_text(l_side_frame.ref.add_child<kee::ui::text>(1,
        kee::color::white,
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.245f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "SONG NAME", false
    )),
    song_name_textbox(l_side_frame.ref.add_child<kee::ui::textbox>(1,
        pos(pos::type::rel, 0.45f),
        pos(pos::type::rel, 0.245f),
        dims(
            dim(dim::type::rel, 0.45f),
            dim(dim::type::rel, 0.03f)
        ),
        false, *this, setup_info.song_name
    )),
    metadata_bg(l_side_frame.ref.add_child<kee::ui::rect>(0,
        kee::color(20, 20, 20),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.5f)
        ),
        false, std::nullopt,
        ui::rect_roundness(ui::rect_roundness::type::rel_w, 0.1f, std::nullopt)
    )),
    metadata_label(l_side_frame.ref.add_child<kee::ui::text>(1,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.55f),
        ui::text_size(ui::text_size::type::rel_h, 0.05f),
        true, assets.font_semi_bold, "METADATA", true
    )),
    mapper_text(l_side_frame.ref.add_child<kee::ui::text>(1,
        kee::color::white,
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.62f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "MAPPER", false
    )),
    mapper_textbox(l_side_frame.ref.add_child<kee::ui::textbox>(1,
        pos(pos::type::rel, 0.45f),
        pos(pos::type::rel, 0.62f),
        dims(
            dim(dim::type::rel, 0.45f),
            dim(dim::type::rel, 0.03f)
        ),
        false, *this, setup_info.mapper
    )),
    level_name_text(l_side_frame.ref.add_child<kee::ui::text>(1,
        kee::color::white,
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.6825f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "LEVEL NAME", false
    )),
    level_name_textbox(l_side_frame.ref.add_child<kee::ui::textbox>(1,
        pos(pos::type::rel, 0.45f),
        pos(pos::type::rel, 0.6825f),
        dims(
            dim(dim::type::rel, 0.45f),
            dim(dim::type::rel, 0.03f)
        ),
        false, *this, setup_info.level_name
    )),
    r_side_frame(add_child<kee::ui::base>(std::nullopt,
        pos(pos::type::rel, 0.55f),
        pos(pos::type::rel, 0.05f),
        dims(
            dim(dim::type::rel, 0.35f),
            dim(dim::type::rel, 0.9f)
        ),
        false
    )),
    difficulty_bg(r_side_frame.ref.add_child<kee::ui::rect>(0,
        kee::color(20, 20, 20),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.4f)
        ),
        false, std::nullopt,
        ui::rect_roundness(ui::rect_roundness::type::rel_w, 0.1f, std::nullopt)
    )),
    difficulty_label(r_side_frame.ref.add_child<kee::ui::text>(1,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.05f),
        ui::text_size(ui::text_size::type::rel_h, 0.05f),
        true, assets.font_semi_bold, "DIFFICULTY", true
    )),
    approach_text(r_side_frame.ref.add_child<kee::ui::text>(1,
        kee::color::white,
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.12f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "APPROACH BEATS", false
    )),
    approach_textbox(r_side_frame.ref.add_child<kee::ui::textbox>(1,
        pos(pos::type::rel, 0.65f),
        pos(pos::type::rel, 0.12f),
        dims(
            dim(dim::type::rel, 0.25f),
            dim(dim::type::rel, 0.03f)
        ),
        false, *this, std::format("{:.2f}", approach_beats)
    )),
    forgiveness_text(r_side_frame.ref.add_child<kee::ui::text>(1,
        kee::color::white,
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.1825f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "BEAT FORGIVENESS", false
    )),
    forgiveness_textbox(r_side_frame.ref.add_child<kee::ui::textbox>(1,
        pos(pos::type::rel, 0.65f),
        pos(pos::type::rel, 0.1825f),
        dims(
            dim(dim::type::rel, 0.25f),
            dim(dim::type::rel, 0.03f)
        ),
        false, *this, std::format("{:.2f}", setup_info.beat_forgiveness)
    )),
    decoration_bg(r_side_frame.ref.add_child<kee::ui::rect>(0,
        kee::color(20, 20, 20),
        pos(pos::type::rel, 0),
        pos(pos::type::rel, 0.5f),
        dims(
            dim(dim::type::rel, 1),
            dim(dim::type::rel, 0.5f)
        ),
        false, std::nullopt,
        ui::rect_roundness(ui::rect_roundness::type::rel_w, 0.1f, std::nullopt)
    )),
    decoration_label(r_side_frame.ref.add_child<kee::ui::text>(1,
        kee::color::white,
        pos(pos::type::rel, 0.5f),
        pos(pos::type::rel, 0.55f),
        ui::text_size(ui::text_size::type::rel_h, 0.05f),
        true, assets.font_semi_bold, "DECORATION", true
    )),
    background_text(r_side_frame.ref.add_child<kee::ui::text>(1,
        kee::color::white,
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.62f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "BACKGROUND", false
    )),
    background_dialog(r_side_frame.ref.add_child<kee::ui::file_dialog>(1,
        pos(pos::type::rel, 0.45f),
        pos(pos::type::rel, 0.62f),
        dims(
            dim(dim::type::rel, 0.45f),
            dim(dim::type::rel, 0.03f)
        ),
        false,
        std::vector<std::string_view>({".png", ".mp4"}),
        [&]() -> std::variant<std::string_view, std::filesystem::path>
        {
            if (setup_info.new_bg_img_path.has_value())
                return setup_info.new_bg_img_path.value().filename();
            else if (root_elem.save_info.has_value() && root_elem.save_info.value().bg_path.has_value())
                return root_elem.save_info.value().bg_path.value();
            else
                return std::string_view("Select an image/video");
        }()
    )),
    key_color_text(r_side_frame.ref.add_child<kee::ui::text>(1,
        kee::color::white,
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.6825f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "KEY COLORS", false
    )),
    key_color_dialog(r_side_frame.ref.add_child<kee::ui::file_dialog>(1,
        pos(pos::type::rel, 0.45f),
        pos(pos::type::rel, 0.6825f),
        dims(
            dim(dim::type::rel, 0.45f),
            dim(dim::type::rel, 0.03f)
        ),
        false,
        std::vector<std::string_view>({".json"}),
        std::string_view("Select a valid JSON file")
    ))
{
    audio_file_dialog.ref.on_success = [&](const std::filesystem::path& song_path)
    {
        this->root_elem.set_song(song_path);
        this->setup_info.new_song_path = song_path;
    };

    audio_file_dialog.ref.on_filter_mismatch = [&]()
    {
        this->root_elem.set_error("Not a valid audio file format!", true);
    };

    artist_textbox.ref.on_string_input = [&](std::string_view new_str) -> bool
    {
        this->setup_info.song_artist = new_str;
        if (this->root_elem.save_info.has_value())
            this->root_elem.save_info.value().save_metadata_needed = true;

        return true;
    };

    song_name_textbox.ref.on_string_input = [&](std::string_view new_str) -> bool
    {
        this->setup_info.song_name = new_str;
        if (this->root_elem.save_info.has_value())
            this->root_elem.save_info.value().save_metadata_needed = true;

        return true;
    };

    mapper_textbox.ref.on_string_input = [&](std::string_view new_str) -> bool
    {
        this->setup_info.mapper = new_str;
        if (this->root_elem.save_info.has_value())
            this->root_elem.save_info.value().save_metadata_needed = true;

        return true;
    };

    level_name_textbox.ref.on_string_input = [&](std::string_view new_str) -> bool
    {
        this->setup_info.level_name = new_str;
        if (this->root_elem.save_info.has_value())
            this->root_elem.save_info.value().save_metadata_needed = true;

        return true;
    };

    approach_textbox.ref.on_string_input = [&](std::string_view new_str) -> bool
    {
        float text_float;

        auto [ptr, ec] = std::from_chars(new_str.data(), new_str.data() + new_str.size(), text_float);
        if (ec != std::errc() || ptr != new_str.data() + new_str.size())
            return false;

        if (text_float < 0.25f || text_float > 16.0f)
        {
            this->root_elem.set_error("Approach beats range: 0.25 to 16", false);
            return false;
        }

        if (this->approach_beats != text_float)
        {
            this->approach_beats = text_float;
            if (this->root_elem.save_info.has_value())
                this->root_elem.save_info.value().save_metadata_needed = true;
        }

        return true;
    };

    forgiveness_textbox.ref.on_string_input = [&](std::string_view new_str) -> bool
    {
        float text_float;

        auto [ptr, ec] = std::from_chars(new_str.data(), new_str.data() + new_str.size(), text_float);
        if (ec != std::errc() || ptr != new_str.data() + new_str.size())
            return false;

        if (text_float < 0.01f || text_float > 1.0f)
        {
            this->root_elem.set_error("Beat forgiveness range: 0.01 to 1", false);
            return false;
        }

        if (this->setup_info.beat_forgiveness != text_float)
        {
            this->setup_info.beat_forgiveness = text_float;
            if (this->root_elem.save_info.has_value())
                this->root_elem.save_info.value().save_metadata_needed = true;
        }

        return true;
    };

    background_dialog.ref.on_success = [&](const std::filesystem::path& bg_path)
    {
        if (bg_path.extension() == ".png")
        {
            this->root_elem.set_bg_img(bg_path);
            this->setup_info.new_bg_img_path = bg_path;
        }
        else
        {
            /* TODO: implement */
        }
    };

    background_dialog.ref.on_filter_mismatch = [&]()
    {
        this->root_elem.set_error("Not an image or a video!", true);
    };

    //key_color_dialog.ref.on_success = [&](const std::filesystem::path& song_path)
    //{
    //    /* TODO: implement */
    //};

    key_color_dialog.ref.on_filter_mismatch = [&]()
    {
        this->root_elem.set_error("Not a JSON file!", true);
    };
}

} // namespace editor
} // namespace scene
} // namespace kee