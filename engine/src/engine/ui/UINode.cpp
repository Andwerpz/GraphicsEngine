#include <engine/ui/UINode.h>
#include <engine/ui/UIRenderer.h>
#include <iostream>

namespace engine::ui {

UINode::UINode(UIRect bounds) :
    parent(std::nullopt),
    bounds(bounds)
{
    // do nothing
}

void UINode::update(float dt) {
    update_impl(dt);
    for(auto& child : children) {
        child->update(dt);
    }
}

void UINode::render(UIRenderer& renderer) {
    // debug
    if(true) {
        auto draw_outline = [&](float inset, glm::vec4 color) {
            std::cout << "DRAW OUTLINE" << std::endl;

            glm::vec2 pos = bounds.pos + glm::vec2(inset);
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

        // debug background rect
        if(parent.has_value()) {
            renderer.draw_rect(bounds, {0.1, 0.1, 0.1, 1.0});
            draw_outline(0, {0.3, 0.3, 0.3, 1.0});
        }

        // debug hovered, captured, focused rects
        if(true) {
            if(hovered)  draw_outline(0.0f, {1.0, 1.0, 0.0, 1.0});   // yellow,     outermost
            if(captured) draw_outline(2.0f, {1.0, 0.0, 1.0, 1.0});   // magenta,    middle
            if(focused)  draw_outline(4.0f, {0.0, 1.0, 1.0, 1.0});   // cyan,       innermost
        }

        float time = std::chrono::duration<float>(std::chrono::steady_clock::now().time_since_epoch()).count();
        float angle = time * 1.0f; // radians/sec
        // renderer.draw_rect({{100 + angle * 50, 500}, {10, 10}}, {1, 1, 1, 1});
        renderer.draw_rect({{100 + sin(angle) * 50, 100}, {100, 100}}, {1, 1, 1, 1});
    }

    render_impl(renderer);
    for(auto& child : children) {
        child->render(renderer);
    }
}

glm::vec2 UINode::absolute_origin() {
    glm::vec2 ret = {0, 0};
    UINode* ptr = this;
    while(true) {
        ret += ptr->bounds.pos;
        if(!ptr->parent.has_value()) break;
        ptr = ptr->parent.value();
    }
    return ret;
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

}