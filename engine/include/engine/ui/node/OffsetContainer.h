#pragma once
#include <algorithm>
#include <engine/ui/node/Container.h>
#include <engine/util/error.h>
#include <iostream>

namespace engine::ui {

// a OffsetContainer is a container that provides an adjustable offset for its children

class OffsetContainer : public UINode {
public:
    OffsetContainer(
        UIContext& context
    ) : UINode(context),
        contents(UINode::emplace_child<Container>())
    {
        // establish defaults
        recompute_offsets();
    }

    template<typename T, typename... Args>
    T& emplace_child(Args&&... args) {
        return contents.emplace_child<T>(std::forward<Args>(args)...);
    }

    void set_offset(glm::vec2 val) {
        offset = val;
        recompute_offsets();
    }
    void increment_offset(glm::vec2 val) {
        offset += val;
        recompute_offsets();
    }
    
    void set_content_dim(glm::vec2 val) {
        if(val.x < 0 || val.y < 0) {
            engine::util::throw_error("Content dim cannot be negative");
        }
        content_dim = val;
        recompute_offsets();
    }
    void set_enable_offset(glm::bvec2 val) {
        enable_offset = val;
        recompute_offsets();
    }
    void set_clamp_offset(glm::bvec2 val) {
        clamp_offset = val;
        recompute_offsets();
    }

    glm::vec2 get_offset() {
        return offset;
    }
    glm::vec2 get_content_dim() {
        return {
            enable_offset.x ? content_dim.x : get_dim().x,
            enable_offset.y ? content_dim.y : get_dim().y
        };
    }
    glm::bvec2 get_enable_offset() {
        return enable_offset;
    }

    glm::vec2 get_max_clamped_offset() {
        return {
            enable_offset.x ? std::max(0.0f, content_dim.x - get_dim().x) : 0.0f,
            enable_offset.y ? std::max(0.0f, content_dim.y - get_dim().y) : 0.0f
        };
    }

private:
    // recomputes offsets of the contents and drag handles
    void recompute_offsets() {
        // clamp scroll offset
        for(int i = 0; i < 2; i++) {
            if(enable_offset[i]) {
                if(clamp_offset[i]) {
                    float max_off = content_dim[i] - get_dim()[i];
                    offset[i] = std::clamp(offset[i], 0.0f, std::max(0.0f, max_off));
                }
            }
            else offset[i] = 0;
        }

        // contents alignment
        glm::vec2 anchor_min(0), anchor_max(1);
        if(enable_offset.x) anchor_max.x = 0;
        if(enable_offset.y) anchor_max.y = 0;
        contents.set_anchor(anchor_min, anchor_max);
        contents.set_offset(-offset);
        contents.set_fixed_dim(content_dim);
    }

    void on_resize() override {
        recompute_offsets();
    }

    // container holding all the contents
    Container& contents;

    // content space offset of the viewport top left
    glm::vec2 offset = {0, 0};
    
    // dimension of the contents
    // only has an affect in axes that are allowed to be offset
    glm::vec2 content_dim = {0, 0};

    // axes that the viewport is allowed to be offset from the content container
    // if an axis is not allowed to be offset, the content dimension exactly matches the viewport
    glm::bvec2 enable_offset = {false, false};

    // if true, then clamps the offset so that the viewport is contained entirely within the contents. 
    // an exception is if the viewport is larger than the contents, in which case the offset
    //   will always be clamped to 0. 
    // only has an effect if we allow offset along the axis we're clamping the offset in
    glm::bvec2 clamp_offset = {false, false};
};

}