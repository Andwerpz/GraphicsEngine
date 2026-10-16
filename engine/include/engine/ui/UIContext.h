#pragma once
#include <chrono>

#include <engine/app/input/InputHandler.h>
#include <engine/app/input/KeyEvent.h>
#include <engine/app/input/MouseButtonEvent.h>
#include <engine/ui/UIRenderer.h>
#include <engine/ui/UINode.h>
#include <engine/ui/node/UIRoot.h>

namespace engine::ui {

// does input handling for a UI node graph

class UIContext {
    using CursorPositionEvent = engine::app::CursorPositionEvent;
    using MouseButtonEvent = engine::app::MouseButtonEvent;
    using ScrollEvent = engine::app::ScrollEvent;
    using KeyEvent = engine::app::KeyEvent;

public:
    UIContext(
        engine::graphics::GraphicsDevice& device,
        engine::graphics::RenderTarget& target
    );

    UIRoot& get_root() {
        return root;
    }

    glm::vec2 get_mouse_pos() {
        return mouse_pos;
    }

    void update();
    void render();

    // external API to send user input to this UIContext
    // the UIContext is responsible for routing these input events to the correct UINode
    void on_cursor_position(const CursorPositionEvent&);
    void on_mouse_button(const MouseButtonEvent&);
    void on_scroll(const ScrollEvent&);
    void on_key(const KeyEvent&);

private:
    // returns the chain of UINodes containing this position, excluding the root
    // pos is relative to the root element
    // parent node will always precede a child node
    std::vector<UINode*> hit_test(glm::vec2 pos);

    // returns the list of UINodes forming the chain from the root to the given node, 
    //   excluding the root
    // if None is provided, then the returned chain will be empty
    std::vector<UINode*> chain_to(std::optional<UINode*> node);

    // updates hover_chain with the current mouse_pos
    void update_hover_chain();

    // updates captured/focused node
    // calls focus/capture gained/lost handlers on the relevant nodes
    // if None is passed, no new captured/focused node is set
    void update_captured_node(std::optional<UINode*> node = std::nullopt);
    void update_focused_node(std::optional<UINode*> node = std::nullopt);

    // UI renderer to submit draw commands to
    UIRenderer renderer;

    // root node managed by the UIContext
    UIRoot root;

    // last time update() was called on this UIContext
    // used to compute dt to give to root node
    // initialized to be the construction time
    std::chrono::nanoseconds last_update_time_ns;

    // position of the mouse as of the last cursor position event
    glm::vec2 mouse_pos = {0, 0};

    // chain of nodes from root to leaf that are currently being hovered, excluding the root
    // hover chain is walked from leaf to root to determine which node receives mouse button and scroll input
    // should be updated every cursor move and context update()
    std::vector<UINode*> hover_chain;

    // node that has captured the mouse (for example, during a mouse drag)
    // a node is captured by being the first node to accept a mouse down input when walking the hover chain
    // a node is uncaptured when the mouse gets released
    // while set, mouse move and button events go to this node only and the hover chain is frozen
    std::optional<UINode*> captured_node;

    // node that is currently focused
    // a node is focused by being the first focusable node on a mouse down input when walking the hover chain
    // a node is unfocused when the mouse next gets pressed
    // while set, keyboard input goes to this node only
    std::optional<UINode*> focused_node;
};

}