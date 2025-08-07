#pragma once

#include <print>
#include <variant>

#include <boost/optional.hpp>

/**
 * Disabling MSVC warnings on raylib's source code.
 */
#ifdef _MSC_VER
    #pragma warning(disable: 4458)
#endif
#include <raylib-cpp.hpp>
#ifdef _MSC_VER
    #pragma warning(default: 4458)
#endif

namespace kee {
namespace ui {

class pos
{
public:
    enum class type;

    pos() = default;
    pos(pos::type pos_type, float val);

    pos::type pos_type;
    float val;
};

enum class pos::type
{
    beg,
    end,
    rel
};

class dim
{
public:
    enum class type;

    dim() = default;
    dim(dim::type dim_type, float val);

    dim::type dim_type;
    float val;
};

enum class dim::type
{
    abs,
    rel,
    /**
     * If only one of the `width` or `height` dimensions are of type `aspect`, its size is scaled from the other
     * axis, the factor stored in its `.val` attribute. If both are `aspect`, then the element fits its parent
     * as much as possible as if it were centered while respecting the provided aspect ratio.
     */
    aspect
};

class dims
{
public:
    dims() = default;
    dims(dim w, dim h);

    dim w;
    dim h;
};

class border
{
public:
    enum class type;

    border(border::type border_type, float val);

    float get_raw_size(const raylib::Rectangle& rect) const;

    border::type border_type;
    float val;
};

enum class border::type
{
    abs,
    rel_w,
    rel_h
};

/**
 * Contractually the first parameter of any non-scene subclass's constructor must be 
 * of type `kee::ui::base&` containing its parent element
 */
class base
{
public:
    base(
        kee::ui::base& parent, 
        kee::ui::pos x, 
        kee::ui::pos y, 
        const std::variant<kee::ui::dims, kee::ui::border>& dimensions, 
        bool centered,
        std::optional<int> z_order,
        bool children_z_order_enabled
    );
    virtual ~base() = default;

    void update(float dt);
    void render() const;

    /**
     * When adding a UI element as a child, pass in the element type's constructor params
     * to `typename... Args` *EXCLUDING* its first constructor parameter of type `kee::ui::base&`!
     * These functions will populate the first parameter for you.
     */
    template <std::derived_from<kee::ui::base> T, typename... Args>
    unsigned int add_child_no_id(Args&&... args);
    template <std::derived_from<kee::ui::base> T, typename... Args>
    void add_child_with_id(unsigned int id, Args&&... args);

    bool has_child(unsigned int id) const;

    std::unique_ptr<kee::ui::base>& child_at(unsigned int id);
    void remove_child(unsigned int key);

    kee::ui::pos x;
    kee::ui::pos y;
    /**
     * `kee::ui::dims` alternative holds regular width and height dimensions.
     * `kee::ui::border` creates an empty border of specified type and sizes the rest of the
     * element within the border as if it were centered
     */
    std::variant<kee::ui::dims, kee::ui::border> dimensions;
    bool centered;

protected:
    /**
     * Scene subclasses do *NOT* specify a `parent`, non-scene subclasses do.
     */
    base(boost::optional<kee::ui::base&> parent);

    virtual void update_element(float dt);
    virtual void render_element() const;

    void set_color(const std::optional<raylib::Color>& color_input);
    raylib::Color get_color() const;

    /**
     * NOTE: We climb up UI hierarchy every time we render a UI element every frame. 
     * Inefficient, if performance is a problem investigate and fix if applicable.
     */
    raylib::Rectangle get_raw_rect() const;
    raylib::Rectangle get_raw_rect_parent() const;

    std::optional<int> z_order;
    bool children_z_order_enabled;

private:
    class ref;

    raylib::Vector2 get_dims(const raylib::Rectangle& parent_raw_rect) const;

    const boost::optional<kee::ui::base&> parent;

    std::optional<raylib::Color> color;
    std::unordered_map<unsigned int, std::unique_ptr<kee::ui::base>> children;

    std::vector<kee::ui::base::ref> z_order_refs;
};

class base::ref
{
public:
    ref(unsigned int id, const kee::ui::base& ui_ref);

    unsigned int get_id() const;
    const kee::ui::base& get() const;

private:
    unsigned int id;
    std::reference_wrapper<const kee::ui::base> ui_ref;
};

/* Template Definitions */

template <std::derived_from<kee::ui::base> T, typename... Args>
unsigned int base::add_child_no_id(Args&&... args)
{
    unsigned int id = 0;
    while (children.contains(id))
        id++;

    children[id] = std::make_unique<T>(*this, std::forward<Args>(args)...);
    if (children_z_order_enabled)
        z_order_refs.push_back(base::ref(id, *children.at(id).get()));

    return id;
}

template <std::derived_from<kee::ui::base> T, typename... Args>
void base::add_child_with_id(unsigned int id, Args&&... args)
{
    auto [_, add_success] = children.try_emplace(id, std::make_unique<T>(*this, std::forward<Args>(args)...));
    if (!add_success)
        throw std::invalid_argument("UI Element already contains child with provided ID!");

    if (children_z_order_enabled)
        z_order_refs.push_back(base::ref(id, *children.at(id).get()));
}

} // namespace ui
} // namespace kee