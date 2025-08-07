#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {
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
    base(const kee::ui::scene::window& window);

private:
    void update_element(float dt) override;

    const kee::ui::scene::window& window;
};

} // namespace scene
} // namespace ui
} // namespace kee