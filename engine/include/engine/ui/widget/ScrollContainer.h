#pragma once
#include <engine/ui/UINode.h>
#include <engine/ui/node/OffsetContainer.h>
#include <engine/ui/node/DragHandle.h>
#include <engine/ui/NodeStateColor.h>

#include <iostream>

namespace engine::ui {

// a ScrollContainer is a scrollable container for other nodes
// it knows how to handle scroll input and provides DragHandles for mouse drag input 

// TODO 
// - implement minimum handle length

class ScrollContainer : public UINode {
    static inline constexpr float DRAG_HANDLE_WIDTH = 10.0f;

public:
    ScrollContainer(
        UIContext& context
    ) : UINode(context),
        contents(UINode::emplace_child<OffsetContainer>()),
        handle {&UINode::emplace_child<DragHandle>(), &UINode::emplace_child<DragHandle>()},
        handle_color {NodeStateColor(*handle[0]), NodeStateColor(*handle[1])}
    {   
        set_should_clip(true);

        // contents
        contents.set_anchor(glm::vec2(0), glm::vec2(1));
        contents.set_clamp_offset(glm::bvec2(1));

        // drag handles
        for(int i = 0; i < 2; i++) {
            handle[i]->set_on_drag_begin(
                [this, axis = i]() -> void {
                    if(!contents.get_enable_offset()[axis]) return;
                    std::cout << "drag start : " << axis << std::endl;
                    drag_handle_offset[axis] = calc_handle_offset(
                        contents.get_content_dim()[axis],
                        get_dim()[axis],
                        contents.get_offset()[axis]
                    );
                }
            );
            handle[i]->set_on_move(
                [this, axis = i](glm::vec2 delta) -> void {
                    std::cout << "drag move : " << axis << std::endl;
                    float cur_handle_off = drag_handle_offset[axis] + delta[axis];
                    float offset = calc_viewport_offset(
                        contents.get_content_dim()[axis],
                        get_dim()[axis],
                        cur_handle_off
                    );
                    set_offset(axis, offset);
                }
            );
        }
        align_handles();
    }

    template<typename T, typename... Args>
    T& emplace_child(Args&&... args) {
        return contents.emplace_child<T>(std::forward<Args>(args)...);
    }

    void set_content_dim(glm::vec2 dim) {
        contents.set_content_dim(dim);
        align_handles();
    }

    void do_horizontal_scrolling(bool val) {
        glm::bvec2 enable = contents.get_enable_offset();
        enable.x = val;
        contents.set_enable_offset(enable);
        align_handles();
    }
    void do_vertical_scrolling(bool val) {
        glm::bvec2 enable = contents.get_enable_offset();
        enable.y = val;
        contents.set_enable_offset(enable);
        align_handles();
    }

private:
    // handle length in pixels
    float calc_handle_length(float content_dim, float viewport_dim) {
        if(content_dim == 0 || content_dim <= viewport_dim) return 0;
        return viewport_dim * (viewport_dim / content_dim);
    }

    // handle offset in pixels from the top/left
    float calc_handle_offset(float content_dim, float viewport_dim, float viewport_off) {
        if(content_dim == 0 || content_dim <= viewport_dim) return 0;
        return viewport_dim * (viewport_off / content_dim);
    }

    // reverse engineer viewport offset from handle offset
    float calc_viewport_offset(float content_dim, float viewport_dim, float handle_off) {
        if(content_dim == 0 || content_dim <= viewport_dim) return 0;
        return content_dim * (handle_off / viewport_dim);
    }

    // manually set the offset and size of the drag handles
    // if they also need to disappear, also handles that
    // uses contents as the source of truth
    void align_handles() {
        for(int i = 0; i < 2; i++) {
            float content_dim = contents.get_content_dim()[i];
            float viewport_dim = get_dim()[i];
            float viewport_off = contents.get_offset()[i];
            bool should_disappear = false;
            if(contents.get_enable_offset()[i]) {
                // if contents can fit entirely within the viewport, no need for drag handle
                if(content_dim <= viewport_dim) {
                    should_disappear = true;
                }
            }
            else should_disappear = true;

            if(should_disappear) {
                // make them have dim {0, 0}
                handle[i]->set_fixed_dim(glm::vec2(0));
                std::cout << "DISAPPEAR : " << i << std::endl;
                std::cout << "DIM : " << content_dim << " " << viewport_dim << std::endl;
            }
            else {
                // compute handle length and offset
                float length = calc_handle_length(content_dim, viewport_dim);
                float offset = calc_handle_offset(content_dim, viewport_dim, viewport_off);
                
                // set dim and offset
                glm::vec2 handle_dim;
                handle_dim[i] = length;
                handle_dim[i ^ 1] = DRAG_HANDLE_WIDTH;
                glm::vec2 handle_offset;
                handle_offset[i] = offset;
                handle_offset[i ^ 1] = get_dim()[i ^ 1] - DRAG_HANDLE_WIDTH;
                handle[i]->set_fixed_dim(handle_dim);
                handle[i]->set_offset(handle_offset);
            }
        }
    }

    void set_offset(int axis, float off) {
        glm::vec2 offset = contents.get_offset();
        offset[axis] = off;
        contents.set_offset(offset);
        align_handles();
    }

    void increment_offset(glm::vec2 delta) {
        contents.increment_offset(delta);
        align_handles();
    }

    void render_overlay(UIRenderer& renderer) override {
        // drag handles
        for(int i = 0; i < 2; i++) {
            if(!contents.get_enable_offset()[i]) continue;

            // render handle
            glm::vec2 pos = handle[i]->get_pos();
            glm::vec2 dim = handle[i]->get_dim();
            renderer.draw_rect({pos, dim}, handle_color[i].get_color());
        }
    }

    bool on_scroll(const ScrollEvent& ev) override {
        constexpr float SCROLL_MULTIPLIER = -15.0f;
        increment_offset(glm::vec2(ev.xoff, ev.yoff) * SCROLL_MULTIPLIER);
        return true;
    }

    void on_resize() override {
        align_handles();
    }

    // offsettable container for contents
    OffsetContainer& contents;

    // {horizontal, vertical} handles
    DragHandle* handle[2];

    // recorded when we start a drag
    glm::vec2 drag_handle_offset;

    // color for the drag handles
    NodeStateColor handle_color[2];
};

}