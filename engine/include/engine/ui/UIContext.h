#pragma once
#include <chrono>
#include <set>

#include <engine/asset/Font.h>
#include <engine/app/input/InputHandler.h>
#include <engine/app/input/KeyEvent.h>
#include <engine/app/input/MouseButtonEvent.h>
#include <engine/ui/UIRenderer.h>
#include <engine/ui/UINode.h>
#include <engine/ui/node/Container.h>

namespace engine::ui {

// handles a UINode graph
// directs mouse/keyboard/scroll inputs to relevant nodes in the graph
// manages asset loading and reuse

class UIContext {
    using CursorPositionEvent = engine::app::CursorPositionEvent;
    using MouseButtonEvent = engine::app::MouseButtonEvent;
    using ScrollEvent = engine::app::ScrollEvent;
    using KeyEvent = engine::app::KeyEvent;
    using Font = engine::asset::Font;

public:
    UIContext(
        engine::graphics::GraphicsDevice& device,
        engine::graphics::RenderTarget& target
    );
    ~UIContext();

    Container& get_root() {
        return root;
    }

    glm::vec2 get_mouse_pos() {
        return mouse_pos;
    }

    void update();
    void render();

    // returns a monotonically increasing 'ui time' in seconds
    // time is relative to when this UIContext was constructed
    // useful for doing time-based animations
    double get_ui_time();

    // external API to send user input to this UIContext
    // the UIContext is responsible for routing these input events to the correct UINode
    void on_cursor_position(const CursorPositionEvent&);
    void on_mouse_button(const MouseButtonEvent&);
    void on_scroll(const ScrollEvent&);
    void on_key(const KeyEvent&);

    // get references to resources managed by the context
    Font& get_font(std::string_view addr, float size);

protected:
    friend class UINode;
    
    // UINodes should call this to notify the context that they want to be destroyed
    // the context will destroy this node at the next safe moment to do so
    void destroy_node(UINode* node);

private:
    // -- INPUT HANDLING --
    // returns the chain of UINodes containing this position, excluding the root
    // pos is relative to the root element
    // parent node will always precede a child node
    std::vector<UINode*> hit_test(glm::vec2 pos);

    // returns the list of UINodes forming the chain from the root to the given node, 
    //   excluding the root
    // if None is provided, then the returned chain will be empty
    std::vector<UINode*> chain_to(std::optional<UINode*> node);

    // updates hover_chain to be the new chain
    // calls mouse enter/leave handlers on the relevant nodes
    void update_hover_chain(std::vector<UINode*> chain);

    // updates captured/focused node
    // calls focus/capture gained/lost handlers on the relevant nodes
    // if None is passed, no new captured/focused node is set
    void update_captured_node(std::optional<UINode*> node = std::nullopt);
    void update_focused_node(std::optional<UINode*> node = std::nullopt);

    // UI renderer to submit draw commands to
    UIRenderer renderer;

    // root node managed by the UIContext
    Container root;

    // time when this UIContext was constructed
    std::chrono::nanoseconds construction_time_ns;

    // last time update() was called on this UIContext
    // used to compute dt to give to root node
    // initialized to be construction_time_ns
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

    // -- ASSETS --
    typedef std::pair<std::string, float> font_key_t;
    std::map<font_key_t, Font> fonts;

    // -- DESTRUCTION --
    // set of nodes that want to be destroyed
    // will destroy these nodes in the next update()
    std::set<UINode*> to_destroy;
};

}