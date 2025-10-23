#include "kee/ui/base.hpp"

#include "kee/scene/base.hpp"
#include "kee/ui/rect.hpp"

namespace kee {
namespace ui {

base::base(
    const kee::ui::required& reqs, 
    kee::pos x, 
    kee::pos y, 
    const std::variant<kee::dims, kee::border>& dimensions, 
    bool centered
) :
    x(x),
    y(y),
    dimensions(dimensions),
    centered(centered),
    reqs(reqs),
    children(std::make_unique<std::multimap<int, std::unique_ptr<kee::ui::base>>>()),
    has_render_priority(false)
{ 
    set_opt_color(raylib::Color::Blank());
}

base::base(base&& other) noexcept :
    x(std::move(other.x)),
    y(std::move(other.y)),
    dimensions(std::move(other.dimensions)),
    centered(other.centered),
    reqs(other.reqs),
    children(std::move(other.children)),
    transitions(std::move(other.transitions)),
    color(std::move(other.color)),
    has_render_priority(other.has_render_priority)
{ 
    for (auto& [_, child] : *children)
        child.get()->reqs.parent = *this;
}

void base::on_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods)
{
    const bool consumed = on_element_key_down(keycode, mods);
    if (!consumed && reqs.parent.has_value())
        reqs.parent.value().on_key_down(keycode, mods);
}

void base::on_key_up(int keycode, magic_enum::containers::bitset<kee::mods> mods)
{
    const bool consumed = on_element_key_up(keycode, mods);
    if (!consumed && reqs.parent.has_value())
        reqs.parent.value().on_key_up(keycode, mods);
}

void base::on_char_press(char c)
{
    const bool consumed = on_element_char_press(c);
    if (!consumed && reqs.parent.has_value())
        reqs.parent.value().on_char_press(c);
}

bool base::on_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods)
{
    for (auto it = children->rbegin(); it != children->rend(); it++)
    {
        std::unique_ptr<kee::ui::base>& child = it->second;
        if (child->on_mouse_down(mouse_pos, is_mouse_l, mods))
            return true;
    }

    return on_element_mouse_down(mouse_pos, is_mouse_l, mods);
}

bool base::on_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods)
{
    for (auto it = children->rbegin(); it != children->rend(); it++)
    {
        std::unique_ptr<kee::ui::base>& child = it->second;
        if (child->on_mouse_up(mouse_pos, is_mouse_l, mods))
            return true;
    }

    return on_element_mouse_up(mouse_pos, is_mouse_l, mods);
}

void base::on_mouse_move(const raylib::Vector2& mouse_pos, magic_enum::containers::bitset<kee::mods> mods)
{
    on_element_mouse_move(mouse_pos, mods);
    for (const auto& [_, child] : *children)
        child->on_mouse_move(mouse_pos, mods);
}

bool base::on_mouse_scroll(float scroll_amount)
{
    for (auto it = children->rbegin(); it != children->rend(); it++)
    {
        std::unique_ptr<kee::ui::base>& child = it->second;
        if (child->on_mouse_scroll(scroll_amount))
            return true;
    }

    return on_element_mouse_scroll(scroll_amount);
}

void base::update(float dt) 
{
    update_element(dt);
    for (const std::unique_ptr<kee::transition_base>& transition : transitions)
        transition->update(dt);

    for (const auto& [_, child] : *children)
        child->update(dt);
}

void base::render() const 
{
    auto it = children->cbegin();
    while (it != children->cend() && it->first < 0)
    {
        if (!it->second->has_render_priority)
            it->second->render();
        it++;
    }

    render_element();
    while (it != children->cend())
    {
        if (!it->second->has_render_priority)
            it->second->render();
        it++;
    }
}

void base::set_opt_color(const std::optional<raylib::Color>& opt_color)
{
    if (!opt_color.has_value() && !reqs.parent.has_value())
        throw std::invalid_argument("Cannot set color source to parent when UI element has no parent!");

    color = opt_color;
}

const std::optional<raylib::Color>& base::get_opt_color() const
{
    return color;
}

raylib::Color base::get_color_from_opt(const std::optional<raylib::Color>& opt_color) const
{
    return opt_color.has_value() ? opt_color.value() : reqs.parent.value().get_color_from_opt(reqs.parent.value().get_opt_color());
}

raylib::Rectangle base::get_raw_rect() const
{
    const raylib::Rectangle parent_raw_rect = reqs.parent.has_value() 
        ? reqs.parent.value().get_raw_rect()
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
    return reqs.parent.value().get_raw_rect();
}

void base::take_render_priority()
{
    std::reference_wrapper<kee::ui::base> scene = *this;
    while (scene.get().reqs.parent.has_value())
        scene = scene.get().reqs.parent.value();

    if (auto scene_ptr = dynamic_cast<kee::scene::base*>(&scene.get()))
    {
        if (!scene_ptr->render_priority.has_value())
        {
            scene_ptr->render_priority.emplace(*this);
            has_render_priority = true;
        }
    }
    else
        throw std::logic_error("Root element is not a scene!");
}

void base::release_render_priority()
{
    std::reference_wrapper<kee::ui::base> scene = *this;
    while (scene.get().reqs.parent.has_value())
        scene = scene.get().reqs.parent.value();

    if (auto scene_ptr = dynamic_cast<kee::scene::base*>(&scene.get()))
    {
        if (scene_ptr->render_priority.has_value() && &scene_ptr->render_priority.value() == this)
        {
            scene_ptr->render_priority.reset();
            has_render_priority = false;
        }
    }
    else
        throw std::logic_error("Root element is not a scene!");
}

base::base(const kee::ui::required& reqs) :
    reqs(reqs),
    children(std::make_unique<std::multimap<int, std::unique_ptr<kee::ui::base>>>()),
    has_render_priority(false)
{ }

bool base::on_element_key_down(
    [[maybe_unused]] int keycode, 
    [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods
) { 
    return false; 
}

bool base::on_element_key_up(
    [[maybe_unused]] int keycode, 
    [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods
) { 
    return false; 
}

bool base::on_element_char_press([[maybe_unused]] char c) { return false; }

bool base::on_element_mouse_down(
    [[maybe_unused]] const raylib::Vector2& mouse_pos, 
    [[maybe_unused]] bool is_mouse_l,
    [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods
) { 
    return false; 
}

bool base::on_element_mouse_up(
    [[maybe_unused]] const raylib::Vector2& mouse_pos, 
    [[maybe_unused]] bool is_mouse_l,
    [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods
) { 
    return false; 
}

void base::on_element_mouse_move(
    [[maybe_unused]] const raylib::Vector2& mouse_pos,
    [[maybe_unused]] magic_enum::containers::bitset<kee::mods> mods
) 
{ }

bool base::on_element_mouse_scroll([[maybe_unused]] float scroll_amount) { return false; }

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

required::required(boost::optional<kee::ui::base&> parent, kee::game& game, kee::global_assets& assets) :
    parent(parent),
    game(game),
    assets(assets)
{ }

} // namespace ui
} // namespace kee