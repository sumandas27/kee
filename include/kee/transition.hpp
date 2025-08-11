#pragma once

#include <optional>
#include <print>

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
 * TODO: open issue on github
 */
class color
{

/* TODO: implement */

/*public:
    color(float r, float g, float b, float a);

private:
    const float r;
    const float g;
    const float b;
    const float a;*/
};

} // namespace kee

#include "transition.ipp"