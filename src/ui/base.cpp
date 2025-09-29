#include "kee/ui/base.hpp"

#include "kee/ui/rect.hpp"

namespace kee {
namespace ui {

base::base(
    const kee::ui::base::required& reqs, 
    kee::pos x, 
    kee::pos y, 
    const std::variant<kee::dims, kee::border>& dimensions, 
    bool centered
) :
    x(x),
    y(y),
    dimensions(dimensions),
    centered(centered),
    assets(reqs.assets),
    parent(reqs.parent)
{ 
    set_opt_color(raylib::Color::Blank());
}

void base::handle_events()
{    
    if (active_child.has_value())
        active_child.value().handle_element_events();
    else
    {
        handle_element_events();
        for (const auto& [_, child] : children)
            child->handle_events();
    }
}

void base::update(float dt) 
{
    update_element(dt);
    for (const std::unique_ptr<kee::transition_base>& transition : transitions)
        transition->update(dt);

    for (const auto& [_, child] : children)
        child->update(dt);
}

void base::render() const 
{
    auto it = children.cbegin();
    while (it != children.cend() && it->first < 0)
    {
        it->second->render();
        it++;
    }

    render_element();
    while (it != children.cend())
    {
        it->second->render();
        it++;
    }
}

void base::set_opt_color(const std::optional<raylib::Color>& opt_color)
{
    if (!opt_color.has_value() && !parent.has_value())
        throw std::invalid_argument("Cannot set color source to parent when UI element has no parent!");

    color = opt_color;
}

const std::optional<raylib::Color>& base::get_opt_color() const
{
    return color;
}

raylib::Color base::get_color_from_opt(const std::optional<raylib::Color>& opt_color) const
{
    return opt_color.has_value() ? opt_color.value() : parent.value().get_color_from_opt(parent.value().get_opt_color());
}

raylib::Rectangle base::get_raw_rect() const
{
    const raylib::Rectangle parent_raw_rect = parent.has_value() 
        ? parent.value().get_raw_rect()
        : raylib::Rectangle(0, 0, 0, 0);

    raylib::Rectangle res;
    res.SetSize(get_dims(parent_raw_rect));

    switch (x.pos_type)
    {
    case pos::type::beg:
        res.x = parent_raw_rect.x + x.val;
        break;
    case pos::type::end:
        res.x = parent_raw_rect.x + parent_raw_rect.width - res.width - x.val;
        break;
    case pos::type::rel:
        res.x = parent_raw_rect.x + parent_raw_rect.width * x.val;
        break;
    }

    switch (y.pos_type)
    {
    case pos::type::beg:
        res.y = parent_raw_rect.y + y.val;
        break;
    case pos::type::end:
        res.y = parent_raw_rect.y + parent_raw_rect.height - res.height - y.val;
        break;
    case pos::type::rel:
        res.y = parent_raw_rect.y + parent_raw_rect.height * y.val;
        break;
    }

    if (centered)
    {
        res.x -= res.width / 2;
        res.y -= res.height / 2;
    }

    return res;
}

raylib::Rectangle base::get_raw_rect_parent() const
{
    return parent.value().get_raw_rect();
}

base::base(const kee::ui::base::required& reqs) :
    assets(reqs.assets),
    parent(reqs.parent)
{ }

void base::handle_element_events() { }

void base::update_element([[maybe_unused]] float dt) { }

void base::render_element() const { }

raylib::Vector2 base::get_dims(const raylib::Rectangle& parent_raw_rect) const
{
    raylib::Vector2 res;
    if (std::holds_alternative<kee::border>(dimensions))
    {
        const kee::border& border = std::get<kee::border>(dimensions);

        float border_size;
        switch (border.border_type)
        {
        case kee::border::type::abs:
            border_size = border.val;
            break;
        case kee::border::type::rel_w:
            border_size = parent_raw_rect.width * border.val;
            break;
        case kee::border::type::rel_h:
            border_size = parent_raw_rect.height * border.val;
            break;
        default:
            std::unreachable();
        }

        res = raylib::Vector2(
            parent_raw_rect.width - 2 * border_size, 
            parent_raw_rect.height - 2 * border_size
        );
        
        return res;
    }
    
    const auto& [w, h] = std::get<kee::dims>(dimensions);
    if (w.dim_type == dim::type::aspect && h.dim_type == dim::type::aspect)
    {
        if (w.val * parent_raw_rect.height >= h.val * parent_raw_rect.width)
        {
            res.x = parent_raw_rect.width;
            res.y = res.x * h.val / w.val;
        }
        else
        {
            res.y = parent_raw_rect.height;
            res.x = res.y * w.val / h.val;
        }

        return res;
    }

    switch (w.dim_type)
    {
    case dim::type::abs:
        res.x = w.val;
        break;
    case dim::type::rel:
        res.x = parent_raw_rect.width * w.val;
        break;
    case dim::type::aspect:
        break;
    }

    switch (h.dim_type)
    {
    case dim::type::abs:
        res.y = h.val;
        break;
    case dim::type::rel:
        res.y = parent_raw_rect.height * h.val;
        break;
    case dim::type::aspect:
        break;
    }

    if (w.dim_type == dim::type::aspect)
        res.x = res.y * w.val;
    if (h.dim_type == dim::type::aspect)
        res.y = res.x * h.val;

    return res;
}

base::required::required(boost::optional<const kee::ui::base&> parent, kee::global_assets& assets) :
    parent(parent),
    assets(assets)
{ }

} // namespace ui
} // namespace kee