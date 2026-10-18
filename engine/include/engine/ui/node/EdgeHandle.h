#pragma once
#include <engine/ui/node/Container.h>
#include <engine/util/error.h>

namespace engine::ui {

// an EdgeHandle knows how to detect when the user drags the edge of its rectangular bounds
// it allows the user to register callbacks to get edge drag events

class EdgeHandle : public Container {
public:
    EdgeHandle(
        UIContext& context
    ) : Container(context) {}

    void set_internal_width(float val) {
        if(val < 0) {
            engine::util::throw_error("internal width must be non-negative");
        }
        internal_width = val;
    }
    void set_external_width(float val) {
        if(val < 0) {
            engine::util::throw_error("external width must be non-negative");
        }
        external_width = val;
    }

    void set_on_drag_begin(std::function<void()> _on_drag_begin) {
        on_drag_begin = _on_drag_begin;
    }
    void set_on_drag_end(std::function<void()> _on_drag_end) {
        on_drag_end = _on_drag_end;
    }
    void set_on_move(std::function<void(float, float, float, float)> _on_move) {
        on_move = _on_move;
    }

private:
    static constexpr uint32_t LEFT_EDGE     = 0b0001;
    static constexpr uint32_t RIGHT_EDGE    = 0b0010;
    static constexpr uint32_t TOP_EDGE      = 0b0100;
    static constexpr uint32_t BOTTOM_EDGE   = 0b1000;
    
    // returns bitmask of edges hit
    uint32_t hit_test_edges(glm::vec2 pos) {
        uint32_t res = 0;

        // handling internal + external width is identical to expanding the bounds outwards
        // on all sides by external width and handling an internal width of (internal + external)
        glm::vec2 dim = get_dim() + glm::vec2(2.0f * external_width);
        pos += glm::vec2(external_width);
        float edge_width = internal_width + external_width;

        // make sure this position is actually inside of the bounds rectangle
        if(0 <= pos.y && pos.y <= dim.y && 0 <= pos.x && pos.x <= dim.x) {
            // left/right
            {
                // see if they can be simultaneously pressed
                if(dim.x < edge_width * 2.0f) {
                    // pick the closer one
                    if(pos.x < (dim.x - pos.x)) res |= LEFT_EDGE;
                    else res |= RIGHT_EDGE;
                }
                else {
                    if(pos.x <= edge_width) res |= LEFT_EDGE;
                    else if((dim.x - pos.x) <= edge_width) res |= RIGHT_EDGE;
                }
            }
            // top/bottom
            {
                // see if they can be simultaneously pressed
                if(dim.y < edge_width * 2.0f) {
                    // pick the closer one
                    if(pos.y < (dim.y - pos.y)) res |= TOP_EDGE;
                    else res |= BOTTOM_EDGE;
                }
                else {
                    if(pos.y <= edge_width) res |= TOP_EDGE;
                    else if((dim.y - pos.y) <= edge_width) res |= BOTTOM_EDGE;
                }
            }
        }

        return res;
    }

    // only return true if pos is over some edge
    bool hit_test(glm::vec2 pos) override {
        return hit_test_edges(pos) != 0;
    }

    void on_cursor_position(const CursorPositionEvent& ev) override {
        if(is_dragging) {
            glm::vec2 cur_mouse = get_absolute_mouse_pos();
            glm::vec2 diff = cur_mouse - drag_start_mouse;
            float left_delta    = (drag_hit_edges & LEFT_EDGE)      ? -diff.x : 0.0f;
            float right_delta   = (drag_hit_edges & RIGHT_EDGE)     ?  diff.x : 0.0f;
            float top_delta     = (drag_hit_edges & TOP_EDGE)       ? -diff.y : 0.0f;
            float bottom_delta  = (drag_hit_edges & BOTTOM_EDGE)    ?  diff.y : 0.0f;
            if(on_move) on_move(left_delta, right_delta, top_delta, bottom_delta);
        }
    }

    bool on_mouse_button(const MouseButtonEvent& ev) override {
        // MB1 down on the header will initiate a window drag
        if(ev.button == MouseButton::Button1 && ev.action == MouseButtonAction::Press) {
            is_dragging = true;
            drag_start_mouse = get_absolute_mouse_pos();
            drag_hit_edges = hit_test_edges(get_absolute_mouse_pos() - get_absolute_pos());
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

    // distance inside of the bounds to consider it an 'edge'
    // if two edges of opposite sides are close enough so that they can be simultaneously pressed
    //   then only the edge that is closer to the mouse will be pressed. 
    float internal_width = 5.0f;

    // distance outside of the bounds to consider it an 'edge'
    float external_width = 0.0f;

    // called when a drag is started
    std::function<void()> on_drag_begin;

    // called when a drag is ended
    std::function<void()> on_drag_end;

    // called when the cursor moves during a drag
    // passes in the total signed delta for each edge since the start of the drag
    // if an edge is not dragged, the total signed delta will always be 0 for that edge
    std::function<void(float, float, float, float)> on_move;

    // if true, the window will adjust its position to be the same relative to the mouse
    bool is_dragging = false;

    // absolute mouse position recorded when a drag starts
    glm::vec2 drag_start_mouse;

    // whether or not the mouse is dragging each edge, captured when a drag starts
    uint32_t drag_hit_edges;
};

}