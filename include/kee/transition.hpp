#pragma once

#include <optional>
#include <print>

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

} // namespace kee

#include "transition.ipp"