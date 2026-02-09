#pragma once

#include <numbers>

#include "kee/ui/base.hpp"

namespace kee {
namespace scene {

class required
{
public:
    required(kee::game& game_ref, kee::global_assets& assets);

    kee::game& game_ref;
    kee::global_assets& assets;
};

/**
 * Contractually the first parameter of any scene subclass's constructor must be of 
 * type `const kee::scene::required&` containing all required data for a scene.
 */
class base : public kee::ui::base
{
public:
    bool on_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;
    bool on_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods) override;

    void render() const override;

    boost::optional<kee::ui::base&> render_priority;

protected:
    base(const kee::scene::required& reqs);
};

} // namespace scene
} // namespace kee