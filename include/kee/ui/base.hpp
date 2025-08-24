#pragma once

#include <algorithm>
#include <variant>

#include <boost/optional.hpp>

#include "kee/global_assets.hpp"
#include "kee/ui_common.hpp"

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

enum class mouse_state
{
    off,
    hot,
    down
};

/**
 * Contractually the first parameter of any non-scene subclass's constructor must be of 
 * type `const kee::base::required&` containing all required references for a UI element.
 */
class base
{
public:
    class required;

    base(
        const kee::ui::base::required& reqs, 
        kee::pos x, 
        kee::pos y, 
        const std::variant<kee::dims, kee::border>& dimensions, 
        const kee::ui::common& common
    );
    base(base&&) = default;
    virtual ~base() = default;

    void handle_events();
    void update(float dt);
    void render() const;

    /**
     * When adding a UI element as a child, pass in the element type's constructor params
     * to `typename... Args` *EXCLUDING* its first constructor parameter of type `kee::ui::base::required&`!
     * These functions will populate the first parameter for you.
     */
    template <std::derived_from<kee::ui::base> T, typename... Args>
    T& add_child(Args&&... args);
    template <std::derived_from<kee::ui::base> T, typename... Args>
    T make_temp_child(Args&&... args) const;

    template <typename T>
    kee::transition<T>& add_transition(const T& default_val);

    void set_opt_color(const std::optional<raylib::Color>& opt_color);
    const std::optional<raylib::Color>& get_opt_color() const;
    raylib::Color get_color_from_opt(const std::optional<raylib::Color>& opt_color) const;

    /**
     * NOTE: We climb up UI hierarchy every time we render a UI element every frame. 
     * Inefficient, if performance is a problem investigate and fix if applicable.
     */
    virtual raylib::Rectangle get_raw_rect() const;
    raylib::Rectangle get_raw_rect_parent() const;

    kee::pos x;
    kee::pos y;
    /**
     * `kee::dims` alternative holds regular width and height dimensions.
     * `kee::border` creates an empty border of specified type on the parent and 
     * sizes the rest of the element within the border as if it were centered.
     */
    std::variant<kee::dims, kee::border> dimensions;
    bool centered;

protected:
    /**
     * Scene subclasses do *NOT* specify a `parent`, non-scene subclasses do.
     */
    base(const kee::ui::base::required& reqs, const kee::ui::common& common);

    virtual void handle_element_events();
    virtual void update_element(float dt);
    virtual void render_element_behind_children() const;
    virtual void render_element_ahead_children() const;

    kee::global_assets& assets;

    boost::optional<kee::ui::base&> active_child;

    std::optional<int> z_order;
    bool children_z_order_enabled;

private:
    raylib::Vector2 get_dims(const raylib::Rectangle& parent_raw_rect) const;

    const boost::optional<const kee::ui::base&> parent;
    std::vector<std::unique_ptr<kee::ui::base>> children;
    std::vector<std::unique_ptr<kee::transition_base>> transitions;

    std::optional<raylib::Color> color;
};

class base::required
{
public:
    required(boost::optional<const kee::ui::base&> parent, kee::global_assets& assets);

    boost::optional<const kee::ui::base&> parent;
    kee::global_assets& assets;
};

} // namespace ui
} // namespace kee

#include "kee/ui/base.ipp"