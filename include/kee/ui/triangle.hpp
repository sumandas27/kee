#pragma once

#include "kee/ui/base.hpp"

namespace kee {
namespace ui {

class triangle final : public kee::ui::base
{
public:
    triangle(
        const kee::ui::required& reqs, 
        const std::optional<raylib::Color>& color, 
        kee::pos x,
        kee::pos y,
        const std::variant<kee::dims, kee::border>& dims,
        bool centered,
        const raylib::Vector2& p0,
        const raylib::Vector2& p1,
        const raylib::Vector2& p2
    );

private:
    void render_element() const override;

    /** 
     * These vectors are relative positions of the vertices in its raw rendering
     * rectangle scaled from 0.0f to 1.0f.
     */
    const raylib::Vector2 p0;
    const raylib::Vector2 p1;
    const raylib::Vector2 p2;
};

} // namespace ui
} // namespace kee