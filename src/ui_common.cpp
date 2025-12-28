#include "kee/ui_common.hpp"

namespace kee {

pos::pos(pos::type pos_type, float val) :
    pos_type(pos_type),
    val(val)
{ }

dim::dim(dim::type dim_type, float val) :
    dim_type(dim_type),
    val(val)
{ }

dims::dims(dim w, dim h) :
    w(w),
    h(h)
{ }

border::border(border::type border_type, float val) :
    border_type(border_type),
    val(val)
{ }

transition_base::transition_base() :
    timer(0.0f)
{ }

void transition_base::update(float dt)
{
    if (timer > 0.0f)
    {
        timer -= dt;
        if (timer <= 0.0f)
            timer = 0.0f;
    }
}

const kee::color color::black = kee::color(0, 0, 0);
const kee::color color::blank = kee::color(0, 0, 0, 0);
const kee::color color::blue_raylib = kee::color(0, 121, 241);
const kee::color color::dark_blue = kee::color(0, 82, 172);
const kee::color color::dark_gray = kee::color(80, 80, 80);
const kee::color color::dark_green = kee::color(0, 117, 44);
const kee::color color::dark_orange = kee::color(255, 140, 0);
const kee::color color::gold = kee::color(255, 203, 0);
const kee::color color::green = kee::color(0, 255, 0);
const kee::color color::green_raylib = kee::color(0, 228, 48);
const kee::color color::red = kee::color(255, 0, 0);
const kee::color color::red_raylib = kee::color(230, 41, 55);
const kee::color color::violet = kee::color(135, 60, 190);
const kee::color color::white = kee::color(255, 255, 255);

raylib::Color color::raylib() const
{
    return raylib::Color(
        static_cast<unsigned char>(r),
        static_cast<unsigned char>(g),
        static_cast<unsigned char>(b),
        static_cast<unsigned char>(a)
    );
}

kee::color color::operator*(float scalar) const
{
    return kee::color(r * scalar, g * scalar, b * scalar, a * scalar);
}

kee::color color::operator+(const kee::color& other) const
{
    return kee::color(r + other.r, g + other.g, b + other.b, a + other.a);
}

beatmap_dir_state::beatmap_dir_state(const std::filesystem::path& path) :
    path(path),
    has_image(std::filesystem::exists(path / "img.png")),
    has_video(std::filesystem::exists(path / "vid.mp4"))
{ }

const std::string_view beatmap_dir_info::standard_img_filename = "img.png";
const std::string_view beatmap_dir_info::standard_vid_filename = "vid.mp4";

const std::filesystem::path beatmap_dir_info::app_data_dir = "test_app_data/";

beatmap_dir_info::beatmap_dir_info(const std::filesystem::path& beatmap_dir_name) :
    dir_state(beatmap_dir_info::app_data_dir / beatmap_dir_name)
{
    const std::filesystem::path json_path = beatmap_dir_info::app_data_dir / beatmap_dir_name / "metadata.json";
    std::ifstream json_stream = std::ifstream(json_path);
    if (!json_stream)
        throw std::runtime_error("Failed to open `metadata.json`");

    const std::string json_contents = std::string(
        std::istreambuf_iterator<char>(json_stream),
        std::istreambuf_iterator<char>()
    );

    const boost::json::value& json_root = boost::json::parse(json_contents);
    if (!json_root.is_object())
        throw std::runtime_error("`metadata.json` root is not an object.");

    const boost::json::object& json_object = json_root.as_object();
    if (!json_object.contains("song_artist") || !json_object.at("song_artist").is_string())
        throw std::runtime_error("`metadata.json` has malformed `song_artist` key.");

    if (!json_object.contains("song_name") || !json_object.at("song_name").is_string())
        throw std::runtime_error("`metadata.json` has malformed `song_name` key.");

    if (!json_object.contains("mapper") || !json_object.at("mapper").is_string())
        throw std::runtime_error("`metadata.json` has malformed `mapper` key.");

    if (!json_object.contains("level_name") || !json_object.at("level_name").is_string())
        throw std::runtime_error("`metadata.json` has malformed `level_name` key.");

    if (!json_object.contains("beat_forgiveness") || !json_object.at("beat_forgiveness").is_double())
        throw std::runtime_error("`metadata.json` has malformed `beat_forgiveness` key.");

    if (!json_object.contains("approach_beats") || !json_object.at("approach_beats").is_double())
        throw std::runtime_error("`metadata.json` has malformed `approach_beats` key.");

    if (!json_object.contains("song_bpm") || !json_object.at("song_bpm").is_double())
        throw std::runtime_error("`metadata.json` has malformed `song_bpm` key.");

    if (!json_object.contains("song_start_offset") || !json_object.at("song_start_offset").is_double())
        throw std::runtime_error("`metadata.json` has malformed `song_start_offset` key.");

    if (!json_object.contains("hit_objects") || !json_object.at("hit_objects").is_object())
        throw std::runtime_error("`metadata.json` has malformed `song_start_offset` key.");

    const boost::json::object& hit_objs = json_object.at("hit_objects").as_object();
    for (const key_pos_data& key_data : kee::key_ui_data)
    {
        const std::string key_str = std::string(1, static_cast<char>(key_data.raylib_key));
        if (!hit_objs.contains(key_str) || !hit_objs.at(key_str).is_array())
            throw std::runtime_error(std::format("`metadata.json` key `hit_objects` is missing `{}` key.", key_str));
    
        const boost::json::array& key_hit_objs = hit_objs.at(key_str).as_array();
        for (const boost::json::value& key_hit_obj : key_hit_objs)
        {
            if (!key_hit_obj.is_object())
                throw std::runtime_error("`metadata.json` key hit object is not a JSON object.");

            const boost::json::object& key_hit_obj_json = key_hit_obj.as_object();
            if (!key_hit_obj_json.contains("beat") || !key_hit_obj_json.at("beat").is_double())
                throw std::runtime_error("`metadata.json` key hit object malformed `beat` key");

            if (!key_hit_obj_json.contains("duration") || !key_hit_obj_json.at("duration").is_double())
                throw std::runtime_error("`metadata.json` key hit object malformed `duration` key");
        }
    }

    const std::filesystem::path song_path = dir_state.path / "song.mp3";
    song = raylib::Music(song_path.string());

    song_name = json_object.at("song_name").as_string();
    song_artist = json_object.at("song_artist").as_string();
    mapper = json_object.at("mapper").as_string();
    level_name = json_object.at("level_name").as_string();

    song_bpm = static_cast<float>(json_object.at("song_bpm").as_double());
    song_start_offset = static_cast<float>(json_object.at("song_start_offset").as_double());

    approach_beats = static_cast<float>(json_object.at("approach_beats").as_double());
    beat_forgiveness = static_cast<float>(json_object.at("beat_forgiveness").as_double());

    keys_json_obj = hit_objs;
}

key_pos_data::key_pos_data(int raylib_key, const raylib::Vector2& relative_pos) :
    raylib_key(raylib_key),
    relative_pos(relative_pos)
{ }

} // namespace kee