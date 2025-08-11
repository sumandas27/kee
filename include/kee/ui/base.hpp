#pragma once

#include <algorithm>
#include <variant>

#include <boost/optional.hpp>

#include "kee/geometry.hpp"
#include "kee/transition.hpp"

namespace kee {
namespace ui {

class common
{
public:
    common(bool centered, std::optional<int> z_order, bool children_z_order_enabled);

    const bool centered;
    const std::optional<int> z_order;
    const bool children_z_order_enabled;
};

/**
 * Contractually the first parameter of any non-scene subclass's constructor must be 
 * of type `kee::ui::base&` containing its parent element
 */
class base
{
public:
    base(
        const kee::ui::base& parent, 
        kee::pos x, 
        kee::pos y, 
        const std::variant<kee::dims, kee::border>& dimensions, 
        const kee::ui::common& common
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
    template <std::derived_from<kee::ui::base> T, typename... Args>
    T make_temp_child(Args&&... args) const;

    bool has_child(unsigned int id) const;

    const std::unique_ptr<kee::ui::base>& child_at(unsigned int id) const;
    std::unique_ptr<kee::ui::base>& child_at(unsigned int id);
    void remove_child(unsigned int key);

    void set_color(const std::optional<raylib::Color>& color_input);
    raylib::Color get_color() const;

    kee::pos x;
    kee::pos y;
    /**
     * `kee::dims` alternative holds regular width and height dimensions.
     * `kee::border` creates an empty border of specified type on the parent and 
     * sizes the rest of the element within the border as if it were centered
     */
    std::variant<kee::dims, kee::border> dimensions;
    bool centered;

    std::unordered_map<unsigned int, std::unique_ptr<kee::transition_base>> transitions;

protected:
    /**
     * Scene subclasses do *NOT* specify a `parent`, non-scene subclasses do.
     */
    base(boost::optional<const kee::ui::base&> parent, const kee::ui::common& common);

    virtual void update_element(float dt);
    virtual void render_element() const;

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

    const boost::optional<const kee::ui::base&> parent;

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

} // namespace ui
} // namespace kee

#include "kee/ui/base.ipp"