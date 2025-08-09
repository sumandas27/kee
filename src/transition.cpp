#include "kee/transition.hpp"

namespace kee {

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

} // namespace kee