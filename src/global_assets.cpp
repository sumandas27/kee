#include "kee/global_assets.hpp"

#include "kee/ui_common.hpp"

namespace kee {
    
performance_stats::performance_stats(unsigned int high_score, unsigned int misses, unsigned int combo, unsigned int best_streak, float acc) :
    high_score(high_score),
    misses(misses),
    combo(combo),
    best_streak(best_streak),
    acc(acc)
{ }

level_ui_assets::level_ui_assets(const std::filesystem::path& beatmap_dir_path, const boost::json::object& user_score_json) :
    beatmap_dir_path(beatmap_dir_path)
{
    const beatmap_dir_info dir_info(beatmap_dir_path);
    if (dir_info.dir_state.has_image)
        img.emplace((dir_info.dir_state.path / beatmap_dir_state::standard_img_filename).string());

    song_name = dir_info.song_name;
    song_artist = dir_info.song_artist;
    mapper = dir_info.mapper;
    level_name = dir_info.level_name;
    total_combo = dir_info.total_combo;

    attempt_count = static_cast<unsigned int>(user_score_json.at("attempts").as_int64());
    if (!user_score_json.at("best").is_null())
    {
        const boost::json::object& best_obj = user_score_json.at("best").as_object();
        const unsigned int high_score = static_cast<unsigned int>(best_obj.at("high_score").as_int64());
        const unsigned int misses = static_cast<unsigned int>(best_obj.at("misses").as_int64());
        const unsigned int combo = static_cast<unsigned int>(best_obj.at("combo").as_int64());
        const unsigned int best_streak = static_cast<unsigned int>(best_obj.at("best_streak").as_int64());
        const float acc = static_cast<float>(best_obj.at("acc").as_double());

        best.emplace(high_score, misses, combo, best_streak, acc);
    }
}

const std::filesystem::path global_assets::app_data_dir = "test_app_data/";
const std::filesystem::path global_assets::user_scores_path = global_assets::app_data_dir  / "user_scores.json";

global_assets::global_assets() :
    font_italic(global_assets::gen_sdf_font("assets/fonts/Montserrat-Italic.ttf")),
    font_light(global_assets::gen_sdf_font("assets/fonts/Montserrat-Light.ttf")),
    font_regular(global_assets::gen_sdf_font("assets/fonts/Montserrat-Regular.ttf")),
    font_semi_bold(global_assets::gen_sdf_font("assets/fonts/Montserrat-SemiBold.ttf")),
    shader_sdf_font(nullptr, "assets/shaders/sdf_text.fs"),
    shader_sdf_rect(nullptr, "assets/shaders/sdf_rect.fs"),
    shader_sdf_triangle(nullptr, "assets/shaders/sdf_triangle.fs"),
    sdf_rect_loc_color_tr(shader_sdf_rect.GetLocation("colorTR")), /* TODO: these aren't needed right ?? */
    sdf_rect_loc_color_br(shader_sdf_rect.GetLocation("colorBR")),
    sdf_rect_loc_color_bl(shader_sdf_rect.GetLocation("colorBL")),
    sdf_rect_loc_color_tl(shader_sdf_rect.GetLocation("colorTL")),
    sdf_rect_loc_size(shader_sdf_rect.GetLocation("size")),
    sdf_rect_loc_roundness_size(shader_sdf_rect.GetLocation("roundnessSize")),
    sdf_rect_loc_outline_color(shader_sdf_rect.GetLocation("outlineColor")),
    sdf_rect_loc_outline_thickness(shader_sdf_rect.GetLocation("outlineThickness")),
    sdf_triangle_loc_color(shader_sdf_triangle.GetLocation("color")),
    sdf_triangle_loc_size(shader_sdf_triangle.GetLocation("size")),
    sdf_triangle_loc_p0(shader_sdf_triangle.GetLocation("p0")),
    sdf_triangle_loc_p1(shader_sdf_triangle.GetLocation("p1")),
    sdf_triangle_loc_p2(shader_sdf_triangle.GetLocation("p2")),
    arrow_png("assets/img/arrow.png"),
    pause_png("assets/img/pause.png"),
    play_png("assets/img/play.png"),
    directory_png("assets/img/directory.png"),
    exit_png("assets/img/exit.png"),
    star_png("assets/img/star.png")
{
    const raylib::Image img_empty(global_assets::texture_empty_size, global_assets::texture_empty_size, raylib::Color::Blank());
    texture_empty = raylib::Texture(img_empty);

    play_assets_future = std::async(std::launch::async, [] 
    {
        std::ifstream performance_json_stream = std::ifstream(global_assets::user_scores_path);
        if (!performance_json_stream)
            throw std::runtime_error("Failed to open `user_scores.json`");

        const std::string performance_json_contents = std::string(
            std::istreambuf_iterator<char>(performance_json_stream),
            std::istreambuf_iterator<char>()
        );

        const boost::json::value& performance_json_root = boost::json::parse(performance_json_contents);
        if (!performance_json_root.is_object())
            throw std::runtime_error("`user_scores.json` root is not an object.");

        const boost::json::object& performance_json_object = performance_json_root.as_object();
        for (const auto& [key, val] : performance_json_object)
        {
            std::size_t entry_id;

            const auto [ptr, ec] = std::from_chars(key.data(), key.data() + key.size(), entry_id);
            if (ec != std::errc() || ptr != key.data() + key.size())
                throw std::runtime_error(std::format("`user_scores.json` key is not an ID: {}", key));

            if (!val.is_object())
                throw std::runtime_error("`user_scores.json` key root is not an object.");

            const boost::json::object& user_score_json = val.as_object();
            if (!user_score_json.contains("attempts") || !user_score_json.at("attempts").is_int64())
                throw std::runtime_error("`user_scores.json` has malformed `attempts` key.");

            if (!user_score_json.contains("attempts"))
                throw std::runtime_error("`user_scores.json` does not have `attempts` key");

            const boost::json::value& best_json = user_score_json.at("best");
            if (!best_json.is_null() && !best_json.is_object())
                throw std::runtime_error("`user_scores.json` key `best` is not `null` or an object.");

            if (best_json.is_null())
                continue;
        
            const boost::json::object& best_obj = best_json.as_object();
            if (!best_obj.contains("high_score") || !best_obj.at("high_score").is_int64())
                throw std::runtime_error("`user_scores.json` key `hold` object has malformed `high_score` key.");

            if (!best_obj.contains("misses") || !best_obj.at("misses").is_int64())
                throw std::runtime_error("`user_scores.json` key `hold` object has malformed `misses` key.");

            if (!best_obj.contains("combo") || !best_obj.at("combo").is_int64())
                throw std::runtime_error("`user_scores.json` key `hold` object has malformed `combo` key.");

            if (!best_obj.contains("best_streak") || !best_obj.at("best_streak").is_int64())
                throw std::runtime_error("`user_scores.json` key `hold` object has malformed `best_streak` key.");

            if (!best_obj.contains("acc") || !best_obj.at("acc").is_double())
                throw std::runtime_error("`user_scores.json` key `hold` object has malformed `acc` key.");
        }

        std::unordered_map<std::size_t, level_ui_assets> res;
        for (const auto& entry : std::filesystem::directory_iterator(global_assets::app_data_dir / "play"))
        {
            if (!entry.is_directory())
                continue;

            std::size_t entry_id;

            const std::string dirname = entry.path().filename().string();
            const auto [ptr, ec] = std::from_chars(dirname.data(), dirname.data() + dirname.size(), entry_id);
            if (ec != std::errc() || ptr != dirname.data() + dirname.size())
                throw std::runtime_error(std::format("Beatmap name not an ID: {}", dirname));

            if (!performance_json_object.contains(dirname))
                throw std::runtime_error(std::format("User does not have score entry for level ID {}", entry_id));

            res.emplace(
                std::piecewise_construct, 
                std::forward_as_tuple(entry_id),
                std::forward_as_tuple(entry.path(), performance_json_object.at(dirname).as_object())
            );
        }

        return res;
    });
}

raylib::Font global_assets::gen_sdf_font(const std::filesystem::path& font_path)
{
    raylib::Font res;
    res.baseSize = 72;
    res.glyphCount = 95;
    res.glyphPadding = 0;
    res.recs = nullptr;

    const raylib::FileData font_file(font_path.string());
    res.glyphs = LoadFontData(font_file.GetData(), font_file.GetBytesRead(), res.baseSize, nullptr, res.glyphCount, FontType::FONT_SDF);

    const raylib::Image font_atlas = GenImageFontAtlas(res.glyphs, &res.recs, res.glyphCount, res.baseSize, res.glyphPadding, 1);
    res.SetTexture(font_atlas.LoadTexture());
    res.GetTexture().SetFilter(TextureFilter::TEXTURE_FILTER_BILINEAR);

    return res;
}

} // namespace kee