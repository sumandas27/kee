#include "kee/ui/base.hpp"

#include <algorithm>

/* TODO: am i getting z order ordering correct ?? */

namespace kee {
namespace ui {

pos::pos(pos::type pos_type, float val) :
    pos_type(pos_type),
    val(val)
{ }

dim::dim(dim::type dim_type, float val) :
    dim_type(dim_type),
    val(val)
{ }

dims::dims(dim w, dim h) :
    w(w),
    h(h)
{ }

border::border(border::type border_type, float val) :
    border_type(border_type),
    val(val)
{ }

float border::get_raw_size(const raylib::Rectangle& rect) const
{
    switch (border_type)
    {
    case kee::ui::border::type::abs:
        return val;
    case kee::ui::border::type::rel_w:
        return rect.width * val;
    case kee::ui::border::type::rel_h:
        return rect.height * val;
    }
}

base::base(
    kee::ui::base& parent, 
    kee::ui::pos x, 
    kee::ui::pos y, 
    const std::variant<kee::ui::dims, kee::ui::border>& dimensions, 
    bool centered,
    std::optional<int> z_order,
    bool children_z_order_enabled
) :
    x(x),
    y(y),
    dimensions(dimensions),
    centered(centered),
    z_order(z_order),
    children_z_order_enabled(children_z_order_enabled),
    parent(parent)
{ 
    set_color(raylib::Color(0, 0, 0, 0));
}

base::base(boost::optional<kee::ui::base&> parent) :
    parent(parent)
{ }

void base::update(float dt) 
{
    update_element(dt);
    
    for (auto& [_, child] : children)
        child->update(dt);

    /* TODO: dont think i need to check for sorted, trusting stl impl to be basically free if sorted, check tho */

    if (children_z_order_enabled)
        std::sort(z_order_refs.begin(), z_order_refs.end(),
            [](const base::ref& l, const base::ref& r) {
                const int l_z_order = l.get().z_order.value_or(0);
                const int r_z_order = r.get().z_order.value_or(0);
                return l_z_order >= r_z_order;
            }
        );
}

void base::render() const 
{ 
    render_element();

    if (!children_z_order_enabled)
        for (const auto& [_, child] : children)
            child->render();
    else
        for (const kee::ui::base::ref& child_ref : z_order_refs)
            child_ref.get().render();
}

bool base::has_child(unsigned int id) const
{
    return children.contains(id);
}

std::unique_ptr<kee::ui::base>& base::child_at(unsigned int id)
{
    return children.at(id);
}

void base::remove_child(unsigned int id)
{
    children.erase(id);
    if (children_z_order_enabled)
        z_order_refs.erase(std::remove_if(z_order_refs.begin(), z_order_refs.end(),
            [id](const base::ref& elem) {
                return elem.get_id() == id;
            }), 
        z_order_refs.end());
}

void base::update_element(float dt) { }

void base::render_element() const { }

void base::set_color(const std::optional<raylib::Color>& color_input)
{
    if (!color_input.has_value() && !parent.has_value())
        throw std::invalid_argument("Cannot set color source to parent when UI element has no parent!");

    color = color_input;
}

raylib::Color base::get_color() const
{
    return color.has_value() ? color.value() : parent.value().get_color();
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

raylib::Vector2 base::get_dims(const raylib::Rectangle& parent_raw_rect) const
{
    raylib::Vector2 res;
    if (std::holds_alternative<kee::ui::border>(dimensions))
    {
        float border_size = std::get<kee::ui::border>(dimensions).get_raw_size(parent_raw_rect);
        res = raylib::Vector2(
            parent_raw_rect.width - 2 * border_size, 
            parent_raw_rect.height - 2 * border_size
        );
        return res;
    }
    
    const auto& [w, h] = std::get<kee::ui::dims>(dimensions);
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

base::ref::ref(unsigned int id, const kee::ui::base& ui_ref) :
    id(id),
    ui_ref(ui_ref)
{ }

unsigned int base::ref::get_id() const
{
    return id;
}

const kee::ui::base& base::ref::get() const
{
    return ui_ref.get();
}

} // namespace ui
} // namespace kee