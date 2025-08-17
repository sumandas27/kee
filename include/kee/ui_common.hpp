#pragma once

#include "kee/global_assets.hpp"

#include <array>
#include <optional>
#include <print>

namespace kee {

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

enum class transition_type
{
    lin,
    exp
};

class transition_base
{
public:
    transition_base();
    virtual ~transition_base() = default;

    void update(float dt);

protected:
    float timer;
};

template <typename T>
class transition final : public transition_base
{
public:
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
 * Unfortunately neither `raylib::Color` nor `raylib::Vector4` has both vector addition and floating-point scalar 
 * multiplication operator overloads (unlike `raylib::Vector2` and `raylib::Vector3` which makes zero sense), 
 * making them unsuitable for our transition system. We use this helper class instead for color interpolation.
 * 
 * NOTE: Remove this when this issue is resolved. Check out this relevant open issue here: 
 *  https://github.com/RobLoach/raylib-cpp/issues/367
 */
class color
{
public:
    static constexpr kee::color dark_orange();
    static constexpr kee::color white();

    constexpr color();
    constexpr color(float r, float g, float b, float a);

    raylib::Color to_color() const;

    kee::color operator*(float scalar) const;
    kee::color operator+(const kee::color& other) const;

private:
    /**
     * Color attributes are scaled from 0.0f to 255.0f
     */
    float r;
    float g;
    float b;
    float a;
};

constexpr kee::color color::dark_orange()
{
    return kee::color(255, 140, 0, 255);
}

constexpr kee::color color::white()
{
    return kee::color(255, 255, 255, 255);
}

constexpr color::color() :
    color(0, 0, 0, 0)
{ }

constexpr color::color(float r, float g, float b, float a) :
    r(r),
    g(g),
    b(b),
    a(a)
{ }

class key_pos_data
{
public:
    key_pos_data(int raylib_key, const raylib::Vector2& relative_pos);

    int raylib_key;
    raylib::Vector2 relative_pos;
};

static constexpr std::size_t key_count = 28;
static const std::array<key_pos_data, key_count> key_ui_data = {
    key_pos_data(KeyboardKey::KEY_Q, raylib::Vector2(0.05f, 0.125f)),
    key_pos_data(KeyboardKey::KEY_W, raylib::Vector2(0.15f, 0.125f)),
    key_pos_data(KeyboardKey::KEY_E, raylib::Vector2(0.25f, 0.125f)),
    key_pos_data(KeyboardKey::KEY_R, raylib::Vector2(0.35f, 0.125f)),
    key_pos_data(KeyboardKey::KEY_T, raylib::Vector2(0.45f, 0.125f)),
    key_pos_data(KeyboardKey::KEY_Y, raylib::Vector2(0.55f, 0.125f)),
    key_pos_data(KeyboardKey::KEY_U, raylib::Vector2(0.65f, 0.125f)),
    key_pos_data(KeyboardKey::KEY_I, raylib::Vector2(0.75f, 0.125f)),
    key_pos_data(KeyboardKey::KEY_O, raylib::Vector2(0.85f, 0.125f)),
    key_pos_data(KeyboardKey::KEY_P, raylib::Vector2(0.95f, 0.125f)),
    key_pos_data(KeyboardKey::KEY_A, raylib::Vector2(0.1f, 0.375f)),
    key_pos_data(KeyboardKey::KEY_S, raylib::Vector2(0.2f, 0.375f)),
    key_pos_data(KeyboardKey::KEY_D, raylib::Vector2(0.3f, 0.375f)),
    key_pos_data(KeyboardKey::KEY_F, raylib::Vector2(0.4f, 0.375f)),
    key_pos_data(KeyboardKey::KEY_G, raylib::Vector2(0.5f, 0.375f)),
    key_pos_data(KeyboardKey::KEY_H, raylib::Vector2(0.6f, 0.375f)),
    key_pos_data(KeyboardKey::KEY_J, raylib::Vector2(0.7f, 0.375f)),
    key_pos_data(KeyboardKey::KEY_K, raylib::Vector2(0.8f, 0.375f)),
    key_pos_data(KeyboardKey::KEY_L, raylib::Vector2(0.9f, 0.375f)),
    key_pos_data(KeyboardKey::KEY_Z, raylib::Vector2(0.15f, 0.625f)),
    key_pos_data(KeyboardKey::KEY_X, raylib::Vector2(0.25f, 0.625f)),
    key_pos_data(KeyboardKey::KEY_C, raylib::Vector2(0.35f, 0.625f)),
    key_pos_data(KeyboardKey::KEY_V, raylib::Vector2(0.45f, 0.625f)),
    key_pos_data(KeyboardKey::KEY_B, raylib::Vector2(0.55f, 0.625f)),
    key_pos_data(KeyboardKey::KEY_N, raylib::Vector2(0.65f, 0.625f)),
    key_pos_data(KeyboardKey::KEY_M, raylib::Vector2(0.75f, 0.625f)),
    key_pos_data(KeyboardKey::KEY_COMMA, raylib::Vector2(0.85f, 0.625f)),
    key_pos_data(KeyboardKey::KEY_SPACE, raylib::Vector2(0.5f, 0.875f))
};

static constexpr float key_border_width = 0.025f;
static constexpr float key_border_parent_h = 0.05f;

} // namespace kee

#include "kee/ui_common.ipp"