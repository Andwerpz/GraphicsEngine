#include <glm/glm.hpp>

#include <engine/graphics/RenderTarget.h>
#include <engine/graphics/GraphicsDevice.h>
#include <engine/ui/UIContext.h>
#include <engine/ui/UIRect.h>
#include <engine/ui/UINode.h>

namespace engine::ui {

UIContext::UIContext(
    engine::graphics::GraphicsDevice& device,
    engine::graphics::RenderTarget& target
) :
    renderer(device, target),
    root(*this, {{0, 0}, {target.get_width(), target.get_height()}})
{
    auto current_time = std::chrono::steady_clock::now();
    auto current_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time.time_since_epoch());
    last_update_time_ns = current_time_ns;
}

void UIContext::update() {
    // update nodes
    auto current_time = std::chrono::steady_clock::now();
    auto current_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time.time_since_epoch());
    std::chrono::nanoseconds delta_time = current_time_ns - last_update_time_ns;
    last_update_time_ns = current_time_ns;
    float dt = (float) ((double) delta_time.count() / 1000000000.0);
    root.update(dt);

    // update hover chain
    if(!captured_node.has_value()) {
        update_hover_chain();
    }
}

void UIContext::render() {
    // gather draw commands
    root.render(renderer);

    // flush draw commands
    renderer.render();
}

void UIContext::on_cursor_position(const CursorPositionEvent& ev) {
    mouse_pos = ev.pos;

    // when captured, mouse input only goes to captured node
    if(captured_node.has_value()) {
        CursorPositionEvent _ev = ev;
        _ev.pos -= captured_node.value()->get_absolute_origin();
        captured_node.value()->on_cursor_position(_ev);
        return;
    }

    // mouse move event goes to hovered node
    update_hover_chain();
    glm::vec2 origin = root.bounds.pos;
    for(int i = 0; i < hover_chain.size(); i++) {
        UINode* node = hover_chain[i];
        origin += node->bounds.pos;
        CursorPositionEvent _ev = ev;
        _ev.pos -= origin;
        node->on_cursor_position(_ev);
    }
}

void UIContext::on_mouse_button(const MouseButtonEvent& ev) {
    using MouseButton = engine::app::MouseButton;
    using MouseButtonAction = engine::app::MouseButtonAction;
    switch(ev.action) {
        case MouseButtonAction::Press: {
            // propagate focus
            bool found_focus = false;
            for(int i = (int) hover_chain.size() - 1; i >= 0; i--) {
                UINode* node = hover_chain[i];
                if(node->focusable()) {
                    found_focus = true;
                    update_focused_node(node);
                    break;
                }
            }
            if(!found_focus) update_focused_node();

            // propagate mouse press 
            for(int i = (int) hover_chain.size() - 1; i >= 0; i--) {
                UINode* node = hover_chain[i];
                if(node->on_mouse_button(ev)) {
                    update_captured_node(node);
                    break;
                }
            }
            break;
        }
        case MouseButtonAction::Release: {
            // update hover chain, it was frozen during mouse drag
            update_hover_chain();

            // send mouse release to captured node
            if(captured_node.has_value()) {
                captured_node.value()->on_mouse_button(ev);
            }

            // reset captured node
            update_captured_node();
            break;
        }
        default:
            assert(false);
    }
}

void UIContext::on_scroll(const ScrollEvent& ev) {
    // scroll input goes to first hovered node that accepts scroll input
    for(int i = (int) hover_chain.size() - 1; i >= 0; i--) {
        UINode* node = hover_chain[i];
        if(node->on_scroll(ev)) {
            break;
        }
    }
}

void UIContext::on_key(const KeyEvent& ev) {
    // keyboard input bubbles up from focused node
    if(focused_node.has_value()) {
        UINode* node = focused_node.value();
        while(true) {
            if(node->on_key(ev)) {
                break;
            }
            if(!node->parent.has_value()) {
                break;
            }
            node = node->parent.value();
        }
    }
}

std::vector<UINode*> UIContext::hit_test(glm::vec2 pos) {
    std::vector<UINode*> res;
    UINode* node_ptr = &root;
    while(true) {
        pos -= node_ptr->bounds.pos;
        std::optional<UINode*> hit = node_ptr->hit_test(pos);
        if(!hit.has_value()) break;
        res.push_back(hit.value());
        node_ptr = hit.value();
    }
    return res;
}

std::vector<UINode*> UIContext::chain_to(std::optional<UINode*> node) {
    std::vector<UINode*> chain;
    if(!node.has_value()) return chain;
    UINode* ptr = node.value();
    while(ptr->parent.has_value()) {
        if(ptr == &root) break;
        chain.push_back(ptr);
        ptr = ptr->parent.value();
    }
    assert(ptr == &root);
    std::reverse(chain.begin(), chain.end());
    return chain;
}

void UIContext::update_hover_chain() {
    // figure out length of shared prefix
    std::vector<UINode*> new_chain = hit_test(mouse_pos);
    int common = 0;
    while(common < hover_chain.size() && common < new_chain.size()) {
        if(hover_chain[common] != new_chain[common]) break;
        common ++;
    }

    // mouse leave events
    for(int i = (int) hover_chain.size() - 1; i >= common; i--) {
        hover_chain[i]->on_mouse_leave();
    }

    // mouse enter events
    hover_chain = std::move(new_chain);
    for(int i = common; i < hover_chain.size(); i++) {
        hover_chain[i]->on_mouse_enter();
    }
}

void UIContext::update_captured_node(std::optional<UINode*> node) {
    if(node == captured_node) return;
    if(captured_node.has_value()) captured_node.value()->on_capture_lost();
    if(node.has_value()) node.value()->on_capture_gained();
    captured_node = node;
}

void UIContext::update_focused_node(std::optional<UINode*> node) {
    if(node == focused_node) return;

    // figure out length of shared prefix
    std::vector<UINode*> old_chain = chain_to(focused_node);
    std::vector<UINode*> new_chain = chain_to(node);
    int common = 0;
    while(common < old_chain.size() && common < new_chain.size()) {
        if(old_chain[common] != new_chain[common]) break;
        common ++;
    }

    // focus lost and leave events
    if(focused_node.has_value()) {
        focused_node.value()->on_focus_lost();
    }
    for(int i = (int) old_chain.size() - 1; i >= common; i--) {
        old_chain[i]->on_focus_leave();
    }

    // focus enter and gained events
    focused_node = node;
    for(int i = common; i < new_chain.size(); i++) {
        new_chain[i]->on_focus_enter();
    }
    if(node.has_value()) {
        node.value()->on_focus_gained();
    }
}
}