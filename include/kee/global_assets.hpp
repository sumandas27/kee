#pragma once

#include <charconv>
#include <filesystem>
#include <future>
#include <optional>

#include <boost/json.hpp>

/**
 * Disabling MSVC warnings on raylib's source code.
 */
#ifdef _MSC_VER
    #pragma warning(disable: 4458)
#endif
#include <raylib-cpp.hpp>
#ifdef _MSC_VER
    #pragma warning(default: 4458)
#endif

namespace kee {

class performance_stats
{
public:
    performance_stats(unsigned int high_score, unsigned int misses, unsigned int combo, unsigned int best_streak, float acc);

    unsigned int high_score;
    unsigned int misses;
    unsigned int combo;
    unsigned int best_streak;
    float acc;
};

class level_ui_assets
{
public:
    level_ui_assets(const std::filesystem::path& beatmap_dir_path, const boost::json::object& user_score_json);

    const std::filesystem::path beatmap_dir_path; /* TODO: ts needed lil bro ????? */

    std::optional<raylib::Image> img;

    std::string song_name;
    std::string song_artist;
    std::string mapper;
    std::string level_name;

    std::optional<performance_stats> best;
    unsigned int total_combo;
    unsigned int attempt_count;
};

class global_assets
{
public:
    static constexpr int texture_empty_size = 1; /* TODO: when do i use ts ?? */

    static const std::filesystem::path app_data_dir; /* TODO FAR: temp */
    static const std::filesystem::path user_scores_path;

    global_assets();

    const raylib::Font font_italic;
    const raylib::Font font_light;
    const raylib::Font font_regular;
    const raylib::Font font_semi_bold;

    raylib::Shader shader_sdf_font;
    raylib::Shader shader_sdf_rect;
    raylib::Shader shader_sdf_triangle;
    raylib::Texture texture_empty;

    const int sdf_rect_loc_color_tr;
    const int sdf_rect_loc_color_br;
    const int sdf_rect_loc_color_bl;
    const int sdf_rect_loc_color_tl;
    const int sdf_rect_loc_size;
    const int sdf_rect_loc_roundness_size;
    const int sdf_rect_loc_outline_color;
    const int sdf_rect_loc_outline_thickness;

    const int sdf_triangle_loc_color;
    const int sdf_triangle_loc_size;
    const int sdf_triangle_loc_p0;
    const int sdf_triangle_loc_p1;
    const int sdf_triangle_loc_p2;

    const raylib::Image arrow_png;
    const raylib::Image pause_png;
    const raylib::Image play_png;
    const raylib::Image directory_png;
    const raylib::Image exit_png;
    const raylib::Image star_png;

    std::future<std::unordered_map<std::size_t, level_ui_assets>> play_assets_future;
    std::unordered_map<std::size_t, level_ui_assets> play_assets;

private:
    static raylib::Font gen_sdf_font(const std::filesystem::path& font_path);
};

} // namespace kee