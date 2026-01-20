#pragma once

#include <array>
#include <expected>
#include <fstream>
#include <optional>
#include <print>

#include <boost/json.hpp>

/**
 * Disabling MSVC warnings on magic_enum's source code.
 */
#ifdef _MSC_VER
    #pragma warning(disable: 4244)
#endif
#include <magic_enum/magic_enum_containers.hpp>
#ifdef _MSC_VER
    #pragma warning(default: 4244)
#endif

#include <nfd.hpp>

#include "kee/global_assets.hpp"

namespace kee {

class game;

class pos
{
public:
    enum class type;

    pos() = default;
    pos(pos::type pos_type, float val);

    pos::type pos_type;
    float val;
};

enum class pos::type
{
    beg,
    end,
    rel
};

class dim
{
public:
    enum class type;

    dim() = default;
    dim(dim::type dim_type, float val);

    dim::type dim_type;
    float val;
};

enum class dim::type
{
    abs,
    rel,
    /**
     * If only one of the `width` or `height` dimensions are of type `aspect`, its size is scaled from the other
     * axis, the factor stored in its `.val` attribute. If both are `aspect`, then the element fits its parent
     * as much as possible as if it were centered while respecting the provided aspect ratio.
     */
    aspect
};

class dims
{
public:
    dims() = default;
    dims(dim w, dim h);

    dim w;
    dim h;
};

class border
{
public:
    enum class type;

    border(border::type border_type, float val);

    border::type border_type;
    float val;
};

enum class border::type
{
    abs,
    rel_w,
    rel_h
};

enum class mouse_state
{
    off,
    hot,
    down
};

enum class mods
{
    ctrl,
    shift
};

enum class transition_type
{
    lin,
    exp,
    inv_exp
};

class transition_base
{
public:
    transition_base();
    transition_base(const transition_base&) = delete;
    virtual ~transition_base() = default;

    transition_base& operator=(const transition_base&) = delete;

    void update(float dt);

protected:
    float timer;
};

template <typename T>
class transition final : public transition_base
{
public:
    static T calculate(T start, T end, float transition_progress, kee::transition_type type);

    transition(const T& default_val);

    T get() const;
    void set(const T& new_default_val);
    void set(const std::optional<T>& new_start, const T& new_end, float new_duration, kee::transition_type new_type);

private:
    std::optional<float> duration;
    kee::transition_type type;

    T default_val;
    T start;
    T end;
};

/**
 * Neither `raylib::Color` nor `raylib::Vector4` has both vector addition and floating-point scalar 
 * multiplication operator overloads (unlike `raylib::Vector2` and `raylib::Vector3` which makes zero sense), 
 * making them unsuitable for our transition system. We use this helper class instead for color interpolation.
 * 
 * NOTE: Remove this when this issue is resolved. Check out this relevant open issue here: 
 *  https://github.com/RobLoach/raylib-cpp/issues/367
 */
class color
{
public:
    static const kee::color black;
    static const kee::color blank;
    static const kee::color blue_raylib;
    static const kee::color dark_blue;
    static const kee::color dark_gray;
    static const kee::color dark_green;
    static const kee::color dark_orange;
    static const kee::color gold;
    static const kee::color green;
    static const kee::color green_raylib;
    static const kee::color red;
    static const kee::color red_raylib;
    static const kee::color violet;
    static const kee::color white;

    constexpr color();
    constexpr color(float r, float g, float b);
    constexpr color(float r, float g, float b, float a);

    raylib::Color raylib() const;

    kee::color operator*(float scalar) const;
    kee::color operator+(const kee::color& other) const;

    bool operator==(const kee::color& other) const = default;

    /**
     * Color attributes are scaled from 0.0f to 255.0f
     */
    float r;
    float g;
    float b;
    float a;
};

constexpr color::color() :
    color(0, 0, 0, 0)
{ }

constexpr color::color(float r, float g, float b) :
    r(r),
    g(g),
    b(b),
    a(255.0f)
{ }

constexpr color::color(float r, float g, float b, float a) :
    r(r),
    g(g),
    b(b),
    a(a)
{ }

class key_decoration
{
public:
    key_decoration(float start_beat, float end_beat, const kee::color& start_color, const kee::color& end_color, kee::transition_type interpolation);

    float start_beat;
    float end_beat;

    kee::color start_color;
    kee::color end_color;

    kee::transition_type interpolation;
};

class beatmap_dir_state
{
public:
    static const std::string_view standard_key_colors_filename;
    static const std::string_view standard_img_filename;
    static const std::string_view standard_vid_filename;
    static const std::string_view standard_custom_hitsound_dirname;

    beatmap_dir_state(const std::filesystem::path& path);

    std::filesystem::path path;
    std::optional<float> video_dir_info;

    bool has_key_colors;
    bool has_image;
    bool has_custom_hitsounds;
};

class beatmap_dir_info
{
public:
    static const std::filesystem::path app_data_dir; /* TODO FAR: temp */

    static std::expected<std::unordered_map<std::string, raylib::Sound>, std::string> validate_custom_hitsounds(const std::filesystem::path& custom_hitsounds_path);
    static std::expected<boost::json::object, std::string> parse_key_colors(const std::filesystem::path& key_color_json_path);
    static std::vector<key_decoration> get_key_decorations(const boost::json::array& key_color_decos);

    beatmap_dir_info(const std::filesystem::path& beatmap_dir_name);

    beatmap_dir_state dir_state;
    
    raylib::Music song;
    std::string song_name;
    std::string song_artist;
    std::string mapper;
    std::string level_name;

    float song_bpm;
    float song_start_offset;

    float approach_beats;
    float beat_forgiveness;

    boost::json::object keys_json_obj;
    std::optional<boost::json::object> key_colors_json_obj;
    
    std::optional<std::unordered_map<std::string, raylib::Sound>>   custom_hitsounds;
};

class key_pos_data
{
public:
    key_pos_data(int raylib_key, const raylib::Vector2& relative_pos);

    int raylib_key;
    raylib::Vector2 relative_pos;
};

static constexpr std::size_t key_count = 31;
static const std::array<key_pos_data, key_count> key_ui_data = {
    key_pos_data(KeyboardKey::KEY_Q, raylib::Vector2(1.0f / 22, 0.125f)),
    key_pos_data(KeyboardKey::KEY_W, raylib::Vector2(3.0f / 22, 0.125f)),
    key_pos_data(KeyboardKey::KEY_E, raylib::Vector2(5.0f / 22, 0.125f)),
    key_pos_data(KeyboardKey::KEY_R, raylib::Vector2(7.0f / 22, 0.125f)),
    key_pos_data(KeyboardKey::KEY_T, raylib::Vector2(9.0f / 22, 0.125f)),
    key_pos_data(KeyboardKey::KEY_Y, raylib::Vector2(11.0f / 22, 0.125f)),
    key_pos_data(KeyboardKey::KEY_U, raylib::Vector2(13.0f / 22, 0.125f)),
    key_pos_data(KeyboardKey::KEY_I, raylib::Vector2(15.0f / 22, 0.125f)),
    key_pos_data(KeyboardKey::KEY_O, raylib::Vector2(17.0f / 22, 0.125f)),
    key_pos_data(KeyboardKey::KEY_P, raylib::Vector2(19.0f / 22, 0.125f)),
    key_pos_data(KeyboardKey::KEY_LEFT_BRACKET, raylib::Vector2(21.0f / 22, 0.125f)),
    key_pos_data(KeyboardKey::KEY_A, raylib::Vector2(1.0f / 11, 0.375f)),
    key_pos_data(KeyboardKey::KEY_S, raylib::Vector2(2.0f / 11, 0.375f)),
    key_pos_data(KeyboardKey::KEY_D, raylib::Vector2(3.0f / 11, 0.375f)),
    key_pos_data(KeyboardKey::KEY_F, raylib::Vector2(4.0f / 11, 0.375f)),
    key_pos_data(KeyboardKey::KEY_G, raylib::Vector2(5.0f / 11, 0.375f)),
    key_pos_data(KeyboardKey::KEY_H, raylib::Vector2(6.0f / 11, 0.375f)),
    key_pos_data(KeyboardKey::KEY_J, raylib::Vector2(7.0f / 11, 0.375f)),
    key_pos_data(KeyboardKey::KEY_K, raylib::Vector2(8.0f / 11, 0.375f)),
    key_pos_data(KeyboardKey::KEY_L, raylib::Vector2(9.0f / 11, 0.375f)),
    key_pos_data(KeyboardKey::KEY_SEMICOLON, raylib::Vector2(10.0f / 11, 0.375f)),
    key_pos_data(KeyboardKey::KEY_Z, raylib::Vector2(3.0f / 22, 0.625f)),
    key_pos_data(KeyboardKey::KEY_X, raylib::Vector2(5.0f / 22, 0.625f)),
    key_pos_data(KeyboardKey::KEY_C, raylib::Vector2(7.0f / 22, 0.625f)),
    key_pos_data(KeyboardKey::KEY_V, raylib::Vector2(9.0f / 22, 0.625f)),
    key_pos_data(KeyboardKey::KEY_B, raylib::Vector2(11.0f / 22, 0.625f)),
    key_pos_data(KeyboardKey::KEY_N, raylib::Vector2(13.0f / 22, 0.625f)),
    key_pos_data(KeyboardKey::KEY_M, raylib::Vector2(15.0f / 22, 0.625f)),
    key_pos_data(KeyboardKey::KEY_COMMA, raylib::Vector2(17.0f / 22, 0.625f)),
    key_pos_data(KeyboardKey::KEY_PERIOD, raylib::Vector2(19.0f / 22, 0.625f)),
    key_pos_data(KeyboardKey::KEY_SPACE, raylib::Vector2(0.5f, 0.875f))
};

static constexpr float key_border_width = 0.035f;
static constexpr float key_border_parent_h = 0.05f;

static constexpr float key_aspect_w = 11;
static constexpr float key_aspect_h = 4;

static constexpr int window_w = 2560;
static constexpr int window_h = 1440;

static constexpr float game_start_bg_opacity = 0.5f;

} // namespace kee

#include "kee/ui_common.ipp"