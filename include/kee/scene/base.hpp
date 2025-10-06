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
    base(const kee::scene::window& window, kee::global_assets& assets);

    bool on_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, bool ctrl_modifier) override;
    bool on_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, bool ctrl_modifier) override;

    void render() const override;

    boost::optional<kee::ui::base&> render_priority;

private:
    void update_element(float dt) override;

    const kee::scene::window& window;
};

} // namespace scene
} // namespace kee