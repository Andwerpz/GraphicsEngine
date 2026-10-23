#pragma once
#include <engine/ui/UINode.h>
#include <engine/ui/node/Container.h>
#include <engine/ui/node/Label.h>
#include <engine/ui/node/DragHandle.h>
#include <engine/ui/node/Button.h>
#include <engine/ui/node/EdgeHandle.h>

namespace engine::ui {

// a Window is a UINode that is meant primarily to display its children 

class Window : public UINode {
    static constexpr glm::vec4 BACKGROUND_COLOR = {0.1, 0.1, 0.1, 1.0};
    static constexpr glm::vec4 HEADER_COLOR = {0.15, 0.15, 0.15, 1.0};
    static constexpr glm::vec4 OUTLINE_COLOR = {0.2, 0.2, 0.2, 1.0};
    static constexpr glm::vec4 ACTIVE_OUTLINE_COLOR = {0.3, 0.3, 0.3, 1.0};
    static constexpr float HEADER_HEIGHT = 20;

    static constexpr float WINDOW_MIN_WIDTH = 50;
    static constexpr float WINDOW_MIN_HEIGHT = 50;

    static constexpr float EDGE_INTERNAL_WIDTH = 1;
    static constexpr float EDGE_EXTERNAL_WIDTH = 5;

public:
    // pos gives the top left position of the window
    // content_dim gives dimensions of the content inside the window
    Window(
        UIContext& context,
        std::string title_text,
        glm::vec2 pos,
        glm::vec2 content_dim
    );

    template<typename T, typename... Args>
    T& emplace_child(Args&&... args) {
        return contents.emplace_child<T>(std::forward<Args>(args)...);
    }

    void do_background_blur(bool val) {
        background_blur = val;
    }

    void set_blur_sigma(uint32_t sigma) {
        blur_sigma = sigma;
    }

private:
    void update_impl(float dt) override;
    void render_content(UIRenderer& renderer) override;
    void render_overlay(UIRenderer& renderer) override;
    
    // we need to extend hit testing to extend outside of the window 
    // this is so that edge grabber can work outside of the window
    bool hit_test(glm::vec2 pos) override;

    // Window should be focusable so that any press within the window will make it active
    bool focusable() const override { return true; }

    void on_cursor_position(const CursorPositionEvent& ev) override;
    bool on_mouse_button(const MouseButtonEvent& ev) override;
    bool on_scroll(const ScrollEvent& ev) override;
    bool on_key(const KeyEvent& ev) override;

    void on_focus_enter_impl() override;

    // draggable window header
    DragHandle& header;

    // title on left side of header
    Label& title;

    // the contents of the window
    Container& contents;

    // draggable window edges
    EdgeHandle& edges;

    // close button on right side of header
    Button& close_button;

    // whether or not to do the background blur
    // by default is off, should enable this if background can be visible. 
    bool background_blur = false;

    // recorded when we start a drag
    glm::vec2 drag_start_pos;
    glm::vec2 drag_start_dim;

    uint32_t blur_sigma = 16;
};

}