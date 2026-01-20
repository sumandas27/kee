#pragma once

namespace kee {

template <typename T>
T transition<T>::calculate(T start, T end, float transition_progress, kee::transition_type type)
{
    float interpolation = 0.0f;

    switch (type)
    {
    case transition_type::lin:
        interpolation = transition_progress;
        break;
    case transition_type::exp:
        interpolation = 1.0f - std::powf(2.0f, -10.0f * transition_progress);
        break;
    case transition_type::inv_exp:
        interpolation = std::powf(2.0f, 10.0f * (transition_progress - 1.f));
        break;
    }

    return end * interpolation + start * (1.0f - interpolation);
}

template <typename T>
transition<T>::transition(const T& default_val) :
    default_val(default_val)
{ }

template <typename T>
T transition<T>::get() const
{
    if (!duration.has_value() || timer == 0.0f)
        return default_val;

    const float transition_progress = 1.0f - (timer / duration.value());
    return transition<T>::calculate(start, end, transition_progress, type);
}

template <typename T>
void transition<T>::set(const T& new_default_val)
{
    timer = 0.0f;
    default_val = new_default_val;
}

template <typename T>
void transition<T>::set(const std::optional<T>& new_start, const T& new_end, float new_duration, kee::transition_type new_type)
{
    if (new_duration < 0.0f)
        throw std::invalid_argument("Input duration must be non-negative!");

    duration = new_duration;
    timer = new_duration;
    type = new_type;

    start = new_start.has_value() ? new_start.value() : default_val;
    end = new_end;
    default_val = new_end;
}

} // namespace kee