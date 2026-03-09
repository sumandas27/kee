#include "kee/global_assets.hpp"

#include "kee/ui_common.hpp"

using namespace std::literals::chrono_literals;

namespace kee {
    
performance_stats::performance_stats(unsigned int high_score, unsigned int misses, unsigned int combo, unsigned int best_streak, float acc) :
    high_score(high_score),
    misses(misses),
    combo(combo),
    best_streak(best_streak),
    acc(acc)
{ }

level_ui_assets::level_ui_assets(const std::filesystem::path& beatmap_dir_path) :
    beatmap_dir_path(beatmap_dir_path)
{
    const beatmap_dir_info dir_info(beatmap_dir_path);
    if (dir_info.dir_state.has_image)
        img.emplace((dir_info.dir_state.path / beatmap_dir_state::standard_img_filename).string());

    song_name = dir_info.song_name;
    song_artist = dir_info.song_artist;
    mapper = dir_info.mapper;
    level_name = dir_info.level_name;

    best = dir_info.best;
    total_combo = dir_info.total_combo;
    attempt_count = dir_info.attempt_count;
}

global_assets::global_assets() :
    font_italic(global_assets::gen_sdf_font("assets/fonts/Montserrat-Italic.ttf")),
    font_light(global_assets::gen_sdf_font("assets/fonts/Montserrat-Light.ttf")),
    font_regular(global_assets::gen_sdf_font("assets/fonts/Montserrat-Regular.ttf")),
    font_semi_bold(global_assets::gen_sdf_font("assets/fonts/Montserrat-SemiBold.ttf")),
    shader_sdf_font(nullptr, "assets/shaders/sdf_text.fs"),
    shader_sdf_rect(nullptr, "assets/shaders/sdf_rect.fs"),
    shader_sdf_triangle(nullptr, "assets/shaders/sdf_triangle.fs"),
    sdf_rect_loc_color_tr(shader_sdf_rect.GetLocation("colorTR")),
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
        std::vector<level_ui_assets> res;
        for (const auto& entry : std::filesystem::directory_iterator(beatmap_dir_info::app_data_dir / "play"))
            if (entry.is_directory())
                res.emplace_back(entry.path());

        return res;
    });
}

void global_assets::update_futures()
{
    if (play_assets_future.valid() && play_assets_future.wait_for(0s) == std::future_status::ready)
        play_assets = play_assets_future.get();
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