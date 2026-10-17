#include <engine/ui/UINode.h>
#include <engine/ui/UIRenderer.h>
#include <engine/ui/UIContext.h>
#include <iostream>

namespace engine::ui {

UINode::UINode(UIContext& context, UIRect bounds) :
    context(context),
    parent(std::nullopt),
    bounds(bounds)
{
    // do nothing
}

UINode::~UINode() {
    // TODO
    // this should probably notify the owning context that it has died
    assert(false);
}

void UINode::update(float dt) {
    update_impl(dt);
    for(auto& child : children) {
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

glm::vec2 UINode::get_absolute_origin() {
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

std::optional<UINode*> UINode::hit_test(glm::vec2 pos) {
    for(int i = (int) children.size() - 1; i >= 0; i--) {
        UINode* child = children[i].get();
        if(!child->hit_testable()) continue;
        glm::vec2 child_dim = child->bounds.dim;
        glm::vec2 child_off = pos - child->bounds.pos;
        if( child_off.x >= 0 && child_off.y >= 0 &&
            child_dim.x >= child_off.x && child_dim.y >= child_off.y
        ) {
            return child;
        }
    }
    return std::nullopt;
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