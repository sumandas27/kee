#pragma once

#include <algorithm>
#include <charconv>
#include <map>
#include <variant>

#include <boost/optional.hpp>

#include "kee/global_assets.hpp"
#include "kee/ui_common.hpp"

namespace kee {
namespace ui {

class base;

template <std::derived_from<kee::ui::base> T>
class handle
{
public:
    handle(const handle&) = delete;
    handle(handle&& other) noexcept;
    ~handle();

    handle& operator=(const handle&) = delete;
    handle& operator=(handle&&) = delete;

    void change_z_order(int new_z_order);

    T& ref;

private:
    friend class base;

    handle(T& ref, std::multimap<int, std::unique_ptr<kee::ui::base>>& multimap, std::multimap<int, std::unique_ptr<kee::ui::base>>::iterator it);

    std::multimap<int, std::unique_ptr<kee::ui::base>>& multimap;
    std::multimap<int, std::unique_ptr<kee::ui::base>>::iterator it;

    bool has_moved;
};

class required
{
public:
    required(boost::optional<kee::ui::base&> parent, kee::game& game_ref, kee::global_assets& assets);
    required(const required&) = default;

    required& operator=(const required&) = delete;

    boost::optional<kee::ui::base&> parent;
    kee::game& game_ref;
    kee::global_assets& assets;
};

/**
 * Contractually the first parameter of any non-scene subclass's constructor must be of 
 * type `const kee::required&` containing all required references for a UI element.
 */
class base
{
public:
    base(
        const kee::ui::required& reqs,
        const kee::pos& x,
        const kee::pos& y,
        const std::variant<kee::dims, kee::border>& dimensions,
        bool centered
    );
    base(const base&) = delete;
    base(base&& other) noexcept;
    virtual ~base() = default;

    base& operator=(const base&) = delete;
    base& operator=(base&&) = delete;

    void on_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods);
    void on_key_up(int keycode, magic_enum::containers::bitset<kee::mods> mods);
    void on_char_press(char c);

    virtual bool on_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods);
    virtual bool on_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods);
    void on_mouse_move(const raylib::Vector2& mouse_pos, magic_enum::containers::bitset<kee::mods> mods);
    bool on_mouse_scroll(float scroll_amount);

    void update(float dt);
    virtual void render() const;

    /**
     * When adding a UI element as a child, pass in the element type's constructor params
     * to `typename... Args` *EXCLUDING* its first constructor parameter of type `kee::ui::required&`!
     * These functions will populate the first parameter for you.
     */
    template <std::derived_from<kee::ui::base> T, typename... Args>
    kee::ui::handle<T> add_child(std::optional<int> z_order, Args&&... args);

    /**
     * You the developer are responsible for manually handling events, updating, and rendering temporary
     * children. Unlike regular children, child references to temporary elements are invalidated when moved.
     * Be mindful when using temporary children event handlers directly!
     */
    template <std::derived_from<kee::ui::base> T, typename... Args>
    T make_temp_child(Args&&... args);

    template <typename T>
    kee::transition<T>& add_transition(const T& default_val);

    /**
     * NOTE: We climb up UI hierarchy every time we render a UI element every frame. 
     * Inefficient, if performance is a problem investigate and fix if applicable.
     */
    raylib::Rectangle get_raw_rect() const;
    raylib::Rectangle get_raw_rect_parent() const;

    void take_render_priority();
    void release_render_priority();

    void take_keyboard_capture();
    void release_keyboard_capture();

    kee::game& game_ref;

    kee::pos x;
    kee::pos y;
    /**
     * `kee::dims` alternative holds regular width and height dimensions.
     * `kee::border` creates an empty border of specified type on the parent and 
     * sizes the rest of the element within the border as if it were centered.
     */
    std::variant<kee::dims, kee::border> dimensions;
    bool centered;

    kee::color color;

protected:
    /**
     * Scene subclasses do *NOT* specify a `parent`, non-scene subclasses do.
     */
    base(kee::game& game_ref, kee::global_assets& assets);

    virtual bool on_element_key_down(int keycode, magic_enum::containers::bitset<kee::mods> mods);
    virtual bool on_element_key_up(int keycode, magic_enum::containers::bitset<kee::mods> mods);
    virtual bool on_element_char_press(char c);

    virtual bool on_element_mouse_down(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods);
    virtual bool on_element_mouse_up(const raylib::Vector2& mouse_pos, bool is_mouse_l, magic_enum::containers::bitset<kee::mods> mods);
    virtual void on_element_mouse_move(const raylib::Vector2& mouse_pos, magic_enum::containers::bitset<kee::mods> mods);
    virtual bool on_element_mouse_scroll(float scroll_amount);

    virtual void update_element(float dt);
    virtual void render_element() const;

    kee::global_assets& assets;

private:
    raylib::Vector2 get_dims(const raylib::Rectangle& parent_raw_rect) const;

    boost::optional<kee::ui::base&> parent;

    /**
     * Outer `unique_ptr` so handle references aren't invalidated on moves.
     * Inner `unique_ptr` so multimap can store subclasses of `kee::ui::base`.
     */
    std::unique_ptr<std::multimap<int, std::unique_ptr<kee::ui::base>>> children;
    std::vector<std::unique_ptr<kee::transition_base>> transitions;

    bool has_render_priority;
};

} // namespace ui
} // namespace kee

#include "kee/ui/base.ipp"