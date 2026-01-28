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

key_decoration::key_decoration(float start_beat, float end_beat, const kee::color& start_color, const kee::color& end_color, kee::transition_type interpolation) :
    start_beat(start_beat),
    end_beat(end_beat),
    start_color(start_color),
    end_color(end_color),
    interpolation(interpolation)
{ }

const std::string_view beatmap_dir_state::standard_key_colors_filename = "key_colors.json";
const std::string_view beatmap_dir_state::standard_img_filename = "img.png";
const std::string_view beatmap_dir_state::standard_vid_filename = "vid.mp4";
const std::string_view beatmap_dir_state::standard_custom_hitsound_dirname = "hitsounds";
const std::string_view beatmap_dir_state::standard_music_filename = "song.mp3";

beatmap_dir_state::beatmap_dir_state(const std::filesystem::path& path) :
    path(path),
    has_key_colors(std::filesystem::exists(path / beatmap_dir_state::standard_key_colors_filename)),
    has_image(std::filesystem::exists(path / beatmap_dir_state::standard_img_filename)),
    has_custom_hitsounds(std::filesystem::is_directory(path / beatmap_dir_state::standard_custom_hitsound_dirname))
{ }

const std::filesystem::path beatmap_dir_info::app_data_dir = "test_app_data/";

std::expected<std::unordered_map<std::string, raylib::Sound>, std::string> beatmap_dir_info::validate_custom_hitsounds(const std::filesystem::path& custom_hitsounds_path)
{
    if (!std::filesystem::is_directory(custom_hitsounds_path))
        return std::unexpected("Not a directory!");

    std::unordered_map<std::string, raylib::Sound> res;
    for (const std::filesystem::directory_entry& hitsound_wav : std::filesystem::directory_iterator(custom_hitsounds_path))
    {
        if (!hitsound_wav.is_regular_file() || hitsound_wav.path().extension() != ".wav")
            return std::unexpected(std::format("'{}' is not a '.wav' file", hitsound_wav.path().string()));

        const std::string wav_filename = hitsound_wav.path().filename().string();
        raylib::Sound& wav = res.insert_or_assign(wav_filename, raylib::Sound(hitsound_wav.path().string())).first->second;

        const float wav_duration = static_cast<float>(wav.frameCount) / wav.stream.sampleRate;
        if (wav_duration >= 1.f)
            return std::unexpected(std::format("'{}' is too long!", wav_filename));
    }

    const bool has_default_filenames =
        res.contains("clap.wav") &&
        res.contains("finish.wav") &&
        res.contains("normal.wav") &&
        res.contains("whistle.wav");

    if (!has_default_filenames)
        return std::unexpected("Doesn't have required default hitsounds.");

    return res;
}

/* WISHLIST: better error handling for `.json` files */

std::expected<boost::json::object, std::string> beatmap_dir_info::parse_key_colors(const std::filesystem::path& key_color_json_path)
{
    std::ifstream key_colors_json_stream = std::ifstream(key_color_json_path);
    if (!key_colors_json_stream)
        return std::unexpected("Failed to open key colors file");

    const std::string key_colors_json_contents = std::string(
        std::istreambuf_iterator<char>(key_colors_json_stream),
        std::istreambuf_iterator<char>()
    );

    const boost::json::value& key_colors_json_root = boost::json::parse(key_colors_json_contents);
    if (!key_colors_json_root.is_object())
        return std::unexpected("`key_colors.json` root is not an object.");

    const boost::json::object& res = key_colors_json_root.as_object();
    for (const key_pos_data& key_data : kee::key_ui_data)
    {
        const std::string key_str = std::string(1, static_cast<char>(key_data.raylib_key));
        if (!res.contains(key_str) || !res.at(key_str).is_array())
            return std::unexpected(std::format("Missing '{}' key.", key_str));
    
        float curr_end_beat = std::numeric_limits<float>::lowest();
        const boost::json::array& key_color_array = res.at(key_str).as_array();
        for (const boost::json::value& key_deco : key_color_array)
        {
            if (!key_deco.is_object())
                return std::unexpected("Not a JSON object.");

            const boost::json::object& key_deco_obj = key_deco.as_object();
            if (!key_deco_obj.contains("start_beat") || !key_deco_obj.at("start_beat").is_double())
                return std::unexpected(std::format("'{}': Malformed 'start_beat' key", key_str));

            if (!key_deco_obj.contains("end_beat") || !key_deco_obj.at("end_beat").is_double())
                return std::unexpected(std::format("'{}': Malformed 'end_beat' key", key_str));

            const float deco_start_beat = static_cast<float>(key_deco_obj.at("start_beat").as_double());
            const float deco_end_beat = static_cast<float>(key_deco_obj.at("end_beat").as_double());
            if (deco_start_beat > deco_end_beat)
                return std::unexpected(std::format("'{}': Start beat appears after end beat!'", key_str));

            if (deco_start_beat < curr_end_beat)
                return std::unexpected(std::format("'{}': Key decorations appear out of order!", key_str));

            curr_end_beat = deco_end_beat;
            if (!key_deco_obj.contains("start_color") || !key_deco_obj.at("start_color").is_object())
                return std::unexpected(std::format("'{}': Malformed 'start_color' key", key_str));

            const boost::json::object& start_color_obj = key_deco_obj.at("start_color").as_object();
            if (!start_color_obj.contains("r") || !start_color_obj.at("r").is_double())
                return std::unexpected(std::format("'{}': 'start_color' has malformed 'r' key", key_str));

            if (!start_color_obj.contains("g") || !start_color_obj.at("g").is_double())
                return std::unexpected(std::format("'{}': 'start_color' has malformed 'g' key", key_str));

            if (!start_color_obj.contains("b") || !start_color_obj.at("b").is_double())
                return std::unexpected(std::format("'{}': 'start_color' has malformed 'b' key", key_str));

            if (!key_deco_obj.contains("end_color") || !key_deco_obj.at("end_color").is_object())
                return std::unexpected(std::format("'{}': Malformed 'end_color' key", key_str));

            const boost::json::object& end_color_obj = key_deco_obj.at("end_color").as_object();
            if (!end_color_obj.contains("r") || !end_color_obj.at("r").is_double())
                return std::unexpected(std::format("'{}': 'end_color' has malformed 'r' key", key_str));

            if (!end_color_obj.contains("g") || !end_color_obj.at("g").is_double())
                return std::unexpected(std::format("'{}': 'end_color' has malformed 'g' key", key_str));

            if (!end_color_obj.contains("b") || !end_color_obj.at("b").is_double())
                return std::unexpected(std::format("'{}': 'end_color' has malformed 'b' key", key_str));
        
            if (!key_deco_obj.contains("interpolation_type") || !key_deco_obj.at("interpolation_type").is_string())
                return std::unexpected(std::format("'{}': Malformed 'interpolation_type' key", key_str));

            const std::string interpolation_type_str = static_cast<std::string>(key_deco_obj.at("interpolation_type").as_string());
            if (!magic_enum::enum_cast<kee::transition_type>(interpolation_type_str).has_value())
                return std::unexpected(std::format("'{}': Malformed 'interpolation_type' key", key_str));
        }
    }

    return res;
}

std::vector<key_decoration> beatmap_dir_info::get_key_decorations(const boost::json::array& key_color_decos)
{
    std::vector<key_decoration> res;
    for (const boost::json::value& key_color_deco : key_color_decos)
    {
        const boost::json::object& key_color_deco_obj = key_color_deco.as_object();
        const float deco_start_beat = static_cast<float>(key_color_deco_obj.at("start_beat").as_double());
        const float deco_end_beat = static_cast<float>(key_color_deco_obj.at("end_beat").as_double());

        const boost::json::object& start_color_json = key_color_deco_obj.at("start_color").as_object();
        const kee::color start_color(
            static_cast<float>(start_color_json.at("r").as_double()),
            static_cast<float>(start_color_json.at("g").as_double()),
            static_cast<float>(start_color_json.at("b").as_double())
        );

        const boost::json::object& end_color_json = key_color_deco_obj.at("end_color").as_object();
        const kee::color end_color(
            static_cast<float>(end_color_json.at("r").as_double()),
            static_cast<float>(end_color_json.at("g").as_double()),
            static_cast<float>(end_color_json.at("b").as_double())
        );

        const std::string interpolation_str = static_cast<std::string>(key_color_deco_obj.at("interpolation_type").as_string());
        kee::transition_type interpolation = magic_enum::enum_cast<kee::transition_type>(interpolation_str).value();

        res.emplace_back(deco_start_beat, deco_end_beat, start_color, end_color, interpolation);
    }

    return res;
}

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

    if (std::filesystem::exists(beatmap_dir_info::app_data_dir / beatmap_dir_name / beatmap_dir_state::standard_vid_filename))
    {
        if (!json_object.contains("video_offset") || !json_object.at("video_offset").is_double())
            throw std::runtime_error("`metadata.json` has malformed `video_offset` key while a video exists.");
    
        dir_state.video_dir_info = static_cast<float>(json_object.at("video_offset").as_double());    
    }

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

            if (!key_hit_obj_json.contains("hitsound") || !key_hit_obj_json.at("hitsound").is_string())
                throw std::runtime_error("`metadata.json` key hit object has malformed start hitsound");

            if (!key_hit_obj_json.contains("hold"))
                throw std::runtime_error("`metadata.json` key hit object does not have `hold` key");

            const boost::json::value& hold_json = key_hit_obj_json.at("hold");
            if (!hold_json.is_null() && !hold_json.is_object())
                throw std::runtime_error("`metadata.json` key hit object `hold` is not `null` or an object.");

            if (hold_json.is_null())
                continue;

            const boost::json::object& hold_obj = hold_json.as_object();
            if (!hold_obj.contains("duration") || !hold_obj.at("duration").is_double())
                throw std::runtime_error("`metadata.json` key hit object has malformed `duration` key.");

            if (!hold_obj.contains("hitsound") || !hold_obj.at("hitsound").is_string())
                throw std::runtime_error("`metadata.json` key hit object has malformed end hitsound.");
        }
    }

    if (dir_state.has_key_colors)
    {
        const auto parsed_json = beatmap_dir_info::parse_key_colors(beatmap_dir_info::app_data_dir / beatmap_dir_name / beatmap_dir_state::standard_key_colors_filename);
        if (parsed_json.has_value())
            key_colors_json_obj = parsed_json.value();
        else
            throw std::runtime_error(parsed_json.error());
    }

    if (dir_state.has_custom_hitsounds)
    {
        auto custom_hitsounds_result = beatmap_dir_info::validate_custom_hitsounds(beatmap_dir_info::app_data_dir / beatmap_dir_name / beatmap_dir_state::standard_custom_hitsound_dirname);
        if (custom_hitsounds_result.has_value())
            custom_hitsounds = std::move(custom_hitsounds_result.value());
        else
            throw std::runtime_error(custom_hitsounds_result.error());
    }

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