#pragma once

#include <optional>
#include <print>

namespace kee {

enum class transition_type
{
    lin,
    quad,
    cubic,
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
    void set(const T& new_start, const T& new_end, float new_duration, kee::transition_type new_type);

private:
    std::optional<float> duration;
    kee::transition_type type;

    T default_val;
    T start;
    T end;
};

} // namespace kee

#include "transition.ipp"