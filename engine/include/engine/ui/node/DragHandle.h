#pragma once
#include <functional>
#include <engine/ui/UINode.h>

namespace engine::ui {

// a DragHandle knows how to get dragged by the user pressing on it and moving the cursor

class DragHandle : public UINode {
public:
    DragHandle(
        UIRect bounds
    ) : UINode(bounds) {}

    void set_on_drag_begin(std::function<void()> _on_drag_begin) {
        on_drag_begin = _on_drag_begin;
    }

    void set_on_drag_end(std::function<void()> _on_drag_end) {
        on_drag_end = _on_drag_end;
    }

    void set_on_move(std::function<void(glm::vec2)> _on_move) {
        on_move = _on_move;
    }

private:
    void on_cursor_position(const CursorPositionEvent& ev) override {
        if(is_dragging) {
            glm::vec2 cur_mouse = get_absolute_mouse_pos();
            glm::vec2 diff = cur_mouse - drag_start_mouse;
            if(on_move) on_move(diff);
        }
    }

    bool on_mouse_button(const MouseButtonEvent& ev) override {
        // MB1 down on the header will initiate a window drag
        if(ev.button == MouseButton::Button1 && ev.action == MouseButtonAction::Press) {
            is_dragging = true;
            drag_start_mouse = get_absolute_mouse_pos();
            if(on_drag_begin) on_drag_begin();
            return true;
        }

        // MB1 up will end a window drag
        if(ev.button == MouseButton::Button1 && ev.action == MouseButtonAction::Release) {
            is_dragging = false;
            if(on_drag_end) on_drag_end();
            return true;
        }

        // consume all mouse events
        return true;
    }

    // called when a drag is started
    std::function<void()> on_drag_begin;

    // called when a drag is ended
    std::function<void()> on_drag_end;

    // called when the cursor moves during a drag
    // passes in the total delta since the start of the drag
    std::function<void(glm::vec2)> on_move;

    // if true, the window will adjust its position to be the same relative to the mouse
    bool is_dragging = false;

    // absolute mouse position recorded when a drag starts
    glm::vec2 drag_start_mouse;
};

}