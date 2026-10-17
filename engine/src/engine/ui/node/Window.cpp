#include <engine/ui/node/Window.h>

namespace engine::ui {

Window::Window(
    UIContext& context,
    std::string title_text,
    glm::vec2 pos,
    glm::vec2 content_dim
) : UINode(context, {pos, {content_dim.x, content_dim.y + HEADER_HEIGHT}}),
    header(UINode::emplace_child<WindowHeader>(
        title_text,
        UIRect {{0, 0}, {content_dim.x, HEADER_HEIGHT}}
    )),
    contents(UINode::emplace_child<Container>(
        UIRect {{0, HEADER_HEIGHT}, content_dim}
    ))
{
    // register drag callbacks
    header.set_on_drag_begin([&]() -> void {
        drag_start_bounds_pos = bounds.pos;
    });
    header.set_on_move([&](glm::vec2 delta) -> void {
        bounds.pos = drag_start_bounds_pos + delta;
    });
}

void Window::update_impl(float dt) {
    // do nothing
}

void Window::render_impl(UIRenderer& renderer) {
    // background blur
    if(background_blur) {
        renderer.draw_blur({{0, 0}, bounds.dim}, blur_sigma);
    }

    // background tint
    constexpr float ALPHA = 0.7;
    constexpr float COLOR = 0.1;
    renderer.draw_rect({{0, 0}, bounds.dim}, {COLOR, COLOR, COLOR, ALPHA});
    
    // header
    renderer.draw_rect({{0, 0}, {bounds.dim.x, HEADER_HEIGHT}}, HEADER_COLOR);
}

void Window::render_overlay_impl(UIRenderer& renderer) {
    // outline
    renderer.draw_rect_outline(
        {{0, 0}, bounds.dim}, 
        is_active() ? ACTIVE_OUTLINE_COLOR : OUTLINE_COLOR
    );
}

void Window::on_cursor_position(const CursorPositionEvent& ev) {
    // do nothing
}

bool Window::on_mouse_button(const MouseButtonEvent& ev) {
    // consume all mouse events
    return true;
}

bool Window::on_scroll(const ScrollEvent& ev) {
    // consume all scroll events
    return true;
}

bool Window::on_key(const KeyEvent& ev) {
    // consume all key events
    return true;
}

void Window::on_focus_enter_impl() {
    // bring this window to front
    bring_to_front();
}

}