#pragma once
#include <optional>
#include <vector>
#include <memory>
#include <utility>

#include <glm/glm.hpp>

#include <engine/app/input/InputHandler.h>
#include <engine/app/input/KeyEvent.h>
#include <engine/app/input/MouseButtonEvent.h>
#include <engine/ui/UIRect.h>
#include <engine/ui/UIRenderer.h>

namespace engine::ui {

// some rectangular region 

class UINode {
    using CursorPositionEvent = engine::app::CursorPositionEvent;
    using MouseButtonEvent = engine::app::MouseButtonEvent;
    using ScrollEvent = engine::app::ScrollEvent;
    using KeyEvent = engine::app::KeyEvent;
    friend class UIContext;

public:
    UINode(UINode&&) = default;
    ~UINode() = default;
 
    void update(float dt);
    void render(UIRenderer& renderer);

    bool is_hovered()   const { return hovered; }
    bool is_captured()  const { return captured; }
    bool is_focused()   const { return focused; }

    // returns the top left bounds position of this node relative to the root origin
    // for now it just walks to the root, TODO figure out how to make this faster
    glm::vec2 absolute_origin();

protected:
    UINode(UIRect bounds);

    virtual void update_impl(float dt) {}
    virtual void render_impl(UIRenderer& renderer) {}

    // adds the child to the node and returns a reference to it
    // this UINode takes ownership of the child
    UINode& add_child(std::unique_ptr<UINode> child) {
        assert(child != nullptr);
        assert(child->parent == std::nullopt);
        UINode& ref = *child;
        children.push_back(std::move(child));
        ref.parent = this;
        return ref;
    }

    // constructs the child, adds it to the node, and returns a reference to it
    template<typename T, typename... Args>
    T& emplace_child(Args&&... args) {
        auto child = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *child;
        add_child(std::move(child));
        return ref;
    }

    // returns the foremost child that contains this position and is hit testable
    // returns None otherwise
    // pos is relative to this nodes bounds
    std::optional<UINode*> hit_test(glm::vec2 pos);

    // overriding this to false is useful to create decorative nodes that don't capture the mouse
    virtual bool hit_testable() const   { return true; }

    // overriding this to true will allow this node to become focusable and receive keyboard input
    virtual bool focusable() const      { return false; }

    // for events except for cursor position, return true if the event was consumed
    virtual void on_cursor_position(const CursorPositionEvent&) {}
    virtual bool on_mouse_button(const MouseButtonEvent&)       { return false; }
    virtual bool on_scroll(const ScrollEvent&)                  { return false; }
    virtual bool on_key(const KeyEvent&)                        { return false; }

    void on_mouse_enter()       { hovered  = true;  on_mouse_enter_impl(); }
    void on_mouse_leave()       { hovered  = false; on_mouse_leave_impl(); }
    void on_capture_gained()    { captured = true;  on_capture_gained_impl(); }
    void on_capture_lost()      { captured = false; on_capture_lost_impl(); }
    void on_focus_gained()      { focused  = true;  on_focus_gained_impl(); }
    void on_focus_lost()        { focused  = false; on_focus_lost_impl(); }

    virtual void on_mouse_enter_impl()      {}
    virtual void on_mouse_leave_impl()      {}
    virtual void on_capture_gained_impl()   {}
    virtual void on_capture_lost_impl()     {}
    virtual void on_focus_gained_impl()     {}
    virtual void on_focus_lost_impl()       {}

    // if this is the root, parent is nullopt
    std::optional<UINode*> parent;

    // all children of this node
    // children will be updated and rendered in the order they appear in this array
    // this implies that children at the end of the array are in the foreground
    std::vector<std::unique_ptr<UINode>> children;

    // rectangular bounds of this node
    // rectangle is relative to the parent node
    UIRect bounds;

    // whether or not this UINode has the mouse hovering over it
    // set on_mouse_enter(), unset on_mouse_leave()
    bool hovered = false;

    // whether or not this UINode is being dragged by the mouse
    // set on_capture_gained(), unset on_capture_lost()
    bool captured = false;

    // whether or not keyboard input is directed towards this UINode
    // set on_focus_gained(), unset on_focus_lost()
    bool focused = false;
};

}