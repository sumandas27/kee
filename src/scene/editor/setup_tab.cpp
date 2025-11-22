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
        }),
        setup_info.from_dir ? std::string_view("song.mp3") : std::string_view("Select a song")
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
        false, *this, setup_info.song_artist
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
        false, *this, setup_info.song_name
    )),
    mapper_text(metadata_bg.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.36f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "MAPPER", false
    )),
    mapper_textbox(metadata_bg.ref.add_child<kee::ui::textbox>(std::nullopt,
        pos(pos::type::rel, 0.45f),
        pos(pos::type::rel, 0.36f),
        dims(
            dim(dim::type::rel, 0.45f),
            dim(dim::type::rel, 0.03f)
        ),
        false, *this, setup_info.mapper
    )),
    level_name_text(metadata_bg.ref.add_child<kee::ui::text>(std::nullopt,
        raylib::Color::White(),
        pos(pos::type::rel, 0.1f),
        pos(pos::type::rel, 0.4225f),
        ui::text_size(ui::text_size::type::rel_h, 0.03f),
        false, assets.font_semi_bold, "LEVEL NAME", false
    )),
    level_name_textbox(metadata_bg.ref.add_child<kee::ui::textbox>(std::nullopt,
        pos(pos::type::rel, 0.45f),
        pos(pos::type::rel, 0.4225f),
        dims(
            dim(dim::type::rel, 0.45f),
            dim(dim::type::rel, 0.03f)
        ),
        false, *this, setup_info.level_name
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
        false, *this, std::format("{:.2f}", approach_beats)
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
        false, *this, std::format("{:.2f}", setup_info.beat_forgiveness)
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
}

} // namespace editor
} // namespace scene
} // namespace kee