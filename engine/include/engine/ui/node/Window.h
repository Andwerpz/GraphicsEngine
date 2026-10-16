#pragma once
#include <engine/ui/UINode.h>
#include <engine/ui/node/Label.h>
#include <engine/ui/node/Container.h>
#include <engine/ui/node/DragHandle.h>

namespace engine::ui {

// a Window is a UINode that is meant primarily to display its children 

class Window : public UINode {
    static constexpr glm::vec4 BACKGROUND_COLOR = {0.1, 0.1, 0.1, 1.0};
    static constexpr glm::vec4 HEADER_COLOR = {0.15, 0.15, 0.15, 1.0};
    static constexpr glm::vec4 OUTLINE_COLOR = {0.2, 0.2, 0.2, 1.0};
    static constexpr glm::vec4 ACTIVE_OUTLINE_COLOR = {0.3, 0.3, 0.3, 1.0};
    static constexpr float HEADER_HEIGHT = 20;

public:
    // pos gives the top left position of the window
    // content_dim gives dimensions of the content inside the window
    Window(
        glm::vec2 pos,
        glm::vec2 content_dim
    );

    template<typename T, typename... Args>
    T& emplace_child(Args&&... args) {
        return contents.emplace_child<T>(std::forward<Args>(args)...);
    }

private:
    void update_impl(float dt) override;
    void render_impl(UIRenderer& renderer) override;
    void render_overlay_impl(UIRenderer& renderer) override;

    // Window should be focusable so that any press within the window will make it active
    bool focusable() const { return true; }

    void on_cursor_position(const CursorPositionEvent& ev) override;
    bool on_mouse_button(const MouseButtonEvent& ev) override;
    bool on_scroll(const ScrollEvent& ev) override;
    bool on_key(const KeyEvent& ev) override;

    void on_focus_enter_impl() override;

    // draggable window header
    DragHandle& header;

    // the contents of the window
    Container& contents;

    // recorded when we start a drag
    glm::vec2 drag_start_bounds_pos;

    // TODO figure out how to set a default font
    // // title rendered in the header
    // Label title;
};

}