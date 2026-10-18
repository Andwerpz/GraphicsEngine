#include <engine/ui/UINode.h>
#include <engine/ui/UIRenderer.h>
#include <engine/ui/UIContext.h>
#include <engine/util/error.h>
#include <iostream>

namespace engine::ui {

UINode::UINode(UIContext& context) :
    context(context),
    parent(std::nullopt),
    bounds({0, 0}, {100, 100})
{
    set_offset(bounds.pos);
    set_fixed_dim(bounds.dim);
}

void UINode::set_anchor(glm::vec2 _anchor_min, std::optional<glm::vec2> _anchor_max) {
    anchor_min = _anchor_min;
    if(_anchor_max.has_value()) anchor_max = _anchor_max.value();
    else anchor_max = _anchor_min;
}

void UINode::set_pivot(glm::vec2 _pivot) {
    pivot = _pivot;
}

void UINode::set_offset(glm::vec2 _offset) {
    offset = _offset;
}

void UINode::set_fixed_dim(glm::vec2 dim) {
    if(dim.x < 0 || dim.y < 0) {
        engine::util::throw_error("UINode fixed dim cannot be negative");
    }
    fixed_dim = dim;
}

void UINode::set_margin_min(glm::vec2 _margin_min) {
    margin_min = _margin_min;
}

void UINode::set_margin_max(glm::vec2 _margin_max) {
    margin_max = _margin_max;
}

void UINode::update(float dt) {
    update_impl(dt);
    for(int i = 0; i < children.size(); i++) {
        auto& child = children[i];
        child->update(dt);
    }
}

void UINode::render(UIRenderer& renderer) {
    renderer.push_origin(bounds.pos);

    // debug
    if(false) {
        auto draw_outline = [&](float inset, glm::vec4 color) {
            glm::vec2 pos = glm::vec2(inset);
            glm::vec2 dim = bounds.dim - glm::vec2(inset * 2.0f);
            if(dim.x < 1.0f || dim.y < 1.0f) return;   // node too small for this inset

            glm::vec2 c0 = pos;
            glm::vec2 c1 = c0;  c1.x += dim.x - 1;
            glm::vec2 c2 = c1;  c2.y += dim.y - 1;
            glm::vec2 c3 = c2;  c3.x -= dim.x - 1;
            renderer.draw_line(c0, c1, 1, color);
            renderer.draw_line(c1, c2, 1, color);
            renderer.draw_line(c2, c3, 1, color);
            renderer.draw_line(c3, c0, 1, color);
        };

        // background rect
        if(parent.has_value()) {
            renderer.draw_rect({{0, 0}, bounds.dim}, {0.1, 0.1, 0.1, 1.0});
            draw_outline(0, {0.3, 0.3, 0.3, 1.0});
        }

        // hovered, captured, focused rects
        if(parent.has_value()) {
            if(hovered)  draw_outline(0.0f, {1.0, 1.0, 0.0, 1.0});   // yellow,     outermost
            if(captured) draw_outline(2.0f, {1.0, 0.0, 1.0, 1.0});   // magenta,    middle
            if(focused)  draw_outline(4.0f, {0.0, 1.0, 1.0, 1.0});   // cyan,       innermost
        }
    }

    render_impl(renderer);
    for(auto& child : children) {
        child->render(renderer);
    }
    render_overlay_impl(renderer);

    renderer.pop_origin();
}

glm::vec2 UINode::get_absolute_pos() {
    glm::vec2 ret = {0, 0};
    UINode* ptr = this;
    while(true) {
        ret += ptr->bounds.pos;
        if(!ptr->parent.has_value()) break;
        ptr = ptr->parent.value();
    }
    return ret;
}

glm::vec2 UINode::get_absolute_mouse_pos() {
    return context.get_mouse_pos();
}

void UINode::bring_to_front() {
    if(parent.has_value()) {
        parent.value()->bring_child_to_front(this);
    }
}

bool UINode::hit_test(glm::vec2 pos) {
    return 
        pos.x >= 0 && pos.y >= 0 && 
        pos.x <= get_dim().x && pos.y <= get_dim().y;
}

std::optional<UINode*> UINode::find_hit_child(glm::vec2 pos) {
    for(int i = (int) children.size() - 1; i >= 0; i--) {
        UINode* child = children[i].get();
        if(!child->hit_testable()) continue;
        if(child->hit_test(pos - child->get_pos())) {
            return child;
        }
    }
    return std::nullopt;
}

void UINode::destroy() {
    context.destroy_node(this);
}

void UINode::resolve_bounds() {
    // resolve this node
    if(parent.has_value()) {
        // non-root node
        glm::vec2 parent_dim = parent.value()->get_dim();
        UIRect next;
        for(int a = 0; a < 2; a++) {
            // resolve anchor + pivot
            if(anchor_min[a] != anchor_max[a]) {
                // using anchor, stretch to fill
                float lo = parent_dim[a] * anchor_min[a] + margin_min[a];
                float hi = parent_dim[a] * anchor_max[a] - margin_max[a];
                next.dim[a] = hi - lo;
                next.pos[a] = lo;
            }
            else {
                // not using anchor, use fixed size + pivot to position
                next.dim[a] = fixed_dim[a];
                next.pos[a] = parent_dim[a] * anchor_min[a]
                            - next.dim[a] * pivot[a];
            }
            next.pos[a] += offset[a];
            next.dim[a] = std::max(next.dim[a], 0.f);
        }
        bounds = next;
    }
    else {
        // root node
        // just use fixed dim and offset
        bounds = UIRect {offset, fixed_dim};
    }

    // resolve child bounds
    for(auto& child : children) {
        child->resolve_bounds();
    }
}

UINode& UINode::add_child(std::unique_ptr<UINode> child) {
    assert(child != nullptr);
    assert(child->parent == std::nullopt);
    assert(&child->context == &context);
    UINode& ref = *child;
    children.push_back(std::move(child));
    ref.parent = this;
    return ref;
}

void UINode::remove_child(UINode* node) {
    // find child
    int child_ind = -1;
    for(int i = 0; i < children.size(); i++) {
        if(children[i].get() == node) {
            assert(child_ind == -1);
            child_ind = i;
        }
    }
    assert(child_ind != -1);

    // delete child 
    children.erase(children.begin() + child_ind);
}

void UINode::bring_child_to_front(UINode* node) {
    // find child
    int child_ind = -1;
    for(int i = 0; i < children.size(); i++) {
        if(children[i].get() == node) {
            assert(child_ind == -1);
            child_ind = i;
        }
    }
    assert(child_ind != -1);

    // place child at back of children list
    assert(children.size() >= 1);
    std::unique_ptr<UINode> child = std::move(children[child_ind]);
    children.erase(children.begin() + child_ind);
    children.push_back(std::move(child));
}

}