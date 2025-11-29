#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace scene {

/* TODO: move to `ui_common.hpp` */

class window
{
public:
    static constexpr int w = 2560;
    static constexpr int h = 1440;

    window();

    raylib::Window impl;
};

class base : public kee::ui::base
{
public:
    base(kee::game& game, kee::global_assets& assets);

    bool on_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;

    void render() const override;

    boost::optional<kee::ui::base&> render_priority;
};

} // namespace scene
} // namespace kee