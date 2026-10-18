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

class UIContext;

class UINode {
    friend class UIContext;

public:
    ~UINode() = default;

    // non-copyable, non-moveable so that addresses of nodes stay constant
    UINode(const UINode&) = delete;
    UINode& operator=(const UINode&) = delete;
    UINode(UINode&&) = delete;
    UINode& operator=(UINode&&) = delete;

    // returns the owning UIContext
    UIContext& get_context() { return context; }

    glm::vec2 get_pos() { return bounds.pos; }
    glm::vec2 get_dim() { return bounds.dim; }

    bool is_hovered()   const { return hovered; }
    bool is_captured()  const { return captured; }
    bool is_active()    const { return active; }
    bool is_focused()   const { return focused; }

    // alignment setters
    void set_anchor(glm::vec2 anchor_min, std::optional<glm::vec2> anchor_max = std::nullopt);
    void set_pivot(glm::vec2 pivot);
    void set_offset(glm::vec2 _offset);
    void set_fixed_dim(glm::vec2 dim);
    void set_margin_min(glm::vec2 margin_min);
    void set_margin_max(glm::vec2 margin_max);

protected:
    using CursorPositionEvent = engine::app::CursorPositionEvent;

    using MouseButtonEvent = engine::app::MouseButtonEvent;
    using MouseButton = engine::app::MouseButton;
    using MouseButtonAction = engine::app::MouseButtonAction;
    
    using ScrollEvent = engine::app::ScrollEvent;

    using KeyEvent = engine::app::KeyEvent;
    using Key = engine::app::Key;
    using KeyAction = engine::app::KeyAction;

    UINode(UIContext& context);
 
    void update(float dt);
    void render(UIRenderer& renderer);

    // returns the top left bounds position of this node relative to the root position
    // for now it just walks to the root, TODO figure out how to make this faster
    // can only be called after init
    glm::vec2 get_absolute_pos();

    // returns the position of the mouse relative to the root position
    glm::vec2 get_absolute_mouse_pos();

    virtual void update_impl(float dt) {}

    // this gets called before all the children get rendered
    virtual void render_impl(UIRenderer& renderer) {}

    // this gets called after all the children get rendered
    virtual void render_overlay_impl(UIRenderer& renderer) {}

    // constructs the child, adds it to the node, and returns a reference to it
    template<typename T, typename... Args>
    T& emplace_child(Args&&... args) {
        auto child = std::make_unique<T>(context, std::forward<Args>(args)...);
        T& ref = *child;
        add_child(std::move(child));
        return ref;
    }

    // move this node to the foreground of its parent's children
    // does nothing if this has no parent
    void bring_to_front();

    // returns whether or not this node contains the given position
    // pos is relative to this nodes bounds
    // by default, just tests against the node bounds
    virtual bool hit_test(glm::vec2 pos);

    // returns the foremost child that contains this position and is hit testable
    // returns None otherwise
    // pos is relative to this nodes bounds
    std::optional<UINode*> find_hit_child(glm::vec2 pos);

    // overriding this to false is useful to create decorative nodes that don't capture the mouse
    virtual bool hit_testable() const   { return true; }

    // overriding this to true will allow this node to become focusable and receive keyboard input
    virtual bool focusable() const      { return false; }

    // for events except for cursor position, return true if the event was consumed
    virtual void on_cursor_position(const CursorPositionEvent&) {}
    virtual bool on_mouse_button(const MouseButtonEvent&)       { return false; }
    virtual bool on_scroll(const ScrollEvent&)                  { return false; }
    virtual bool on_key(const KeyEvent&)                        { return false; }

    void on_mouse_enter()           { hovered  = true;  on_mouse_enter_impl(); }
    void on_mouse_leave()           { hovered  = false; on_mouse_leave_impl(); }
    void on_capture_gained()        { captured = true;  on_capture_gained_impl(); }
    void on_capture_lost()          { captured = false; on_capture_lost_impl(); }
    void on_focus_enter()           { active   = true;  on_focus_enter_impl(); }
    void on_focus_leave()           { active   = false; on_focus_leave_impl(); }
    void on_focus_gained()          { focused  = true;  on_focus_gained_impl(); }
    void on_focus_lost()            { focused  = false; on_focus_lost_impl(); }

    virtual void on_mouse_enter_impl()      {}
    virtual void on_mouse_leave_impl()      {}
    virtual void on_capture_gained_impl()   {}
    virtual void on_capture_lost_impl()     {}
    virtual void on_focus_enter_impl()      {}
    virtual void on_focus_leave_impl()      {}
    virtual void on_focus_gained_impl()     {}
    virtual void on_focus_lost_impl()       {}

    // tells the context to destroy this node
    void destroy();

    // resolves this nodes bounds and recursively resolves all child bounds 
    void resolve_bounds();

    // these get called when bounds get changed
    // TODO decide when to call these. 
    //   Calling them inside resolve_bounds() doesn't seem like too good of an idea because
    //   they would be operating on a tree that is currently being modified
    //   maybe we can give them to the context to call later?
    //   don't really want the context to have to handle the internal workings of resizing
    // virtual void on_reposition() {};
    // virtual void on_resize() {};

private:
    // adds the child to the node and returns a reference to it
    // this UINode takes ownership of the child
    UINode& add_child(std::unique_ptr<UINode> child);

    // removes the child from this node and deletes it
    void remove_child(UINode* child);

    // places the given child in front of all other children of this node
    void bring_child_to_front(UINode* child);

    // owning UIContext
    UIContext& context;

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

    // whether or not the focused node is in this nodes subtree
    // focused implies active, but not the other way around
    // set on_focus_enter(), unset on_focus_leave()
    bool active = false;

    // whether or not keyboard input is directed towards this UINode
    // set on_focus_gained(), unset on_focus_lost()
    bool focused = false;

    // represents a rectangle in normalized parent coordinates, 
    //   so (0, 0) is the parent origin, (1, 1) is the parent bottom right corner
    // for an axis, if min != max, this nodes bounds should stretch to fill
    //   if min == max, then it just gives a reference point for the pivot
    glm::vec2 anchor_min = {0, 0};
    glm::vec2 anchor_max = {0, 0};

    // represents a point in normalized coordinates for this node
    // for an axis, if anchor min == max, then pivot tells what coordinate of this node 
    //   that needs to overlap with the anchor coordinate in the parent
    glm::vec2 pivot = {0, 0};

    // pixel offset applied to the position after anchor and pivot are resolved
    glm::vec2 offset;

    // manually set size in pixels, only applies along an axis when anchors are the same
    glm::vec2 fixed_dim;

    // pixel insets from each anchored edge, so positive margin shrinks size 
    // only active when anchoring is active for that axis
    glm::vec2 margin_min = {0, 0};   // left/top
    glm::vec2 margin_max = {0, 0};   // right/bottom
};

}