#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace scene {

class window
{
public:
    window();

    raylib::Window impl;
};

class base : public kee::ui::base
{
public:
    base(const kee::scene::window& window);

private:
    void update_element(float dt) override;

    const kee::scene::window& window;
};

} // namespace scene
} // namespace kee