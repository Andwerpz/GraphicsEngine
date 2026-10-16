#include <engine/ui/node/Button.h>
#include <engine/app/input/MouseButtonEvent.h>
#include <iostream>

namespace engine::ui {

Button::Button(
    std::function<void()> callback,
    UIRect bounds
) : UINode(bounds),
    callback(callback)
{
    // do nothing
}

void Button::update_impl(float dt) {
    // do nothing
}

void Button::render_impl(UIRenderer& renderer) {
    glm::vec4 color;
    if(is_captured()) {
        color = PRESSED_COLOR;
    }
    else if(is_hovered()) {
        color = HOVERED_COLOR;
    }
    else {
        color = NORMAL_COLOR;
    }
    renderer.draw_rect({{0, 0}, bounds.dim}, color);
}

bool Button::on_mouse_button(const MouseButtonEvent& ev) {
    if(ev.action == engine::app::MouseButtonAction::Release && is_hovered()) {
        if(callback) callback();
    }
    return true;
}

}