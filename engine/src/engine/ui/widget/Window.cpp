#include <engine/ui/widget/Window.h>

namespace engine::ui {

Window::Window(
    UIContext& context,
    std::string title_text,
    glm::vec2 pos,
    glm::vec2 content_dim
) : UINode(context),
    header(UINode::emplace_child<DragHandle>()),
    title(header.emplace_child<Label>(
        title_text,
        "engine://res/fonts/consolas.ttf", 12.0
    )),
    contents(UINode::emplace_child<Container>()),
    edges(UINode::emplace_child<EdgeHandle>()),
    close_button(header.emplace_child<Button>())
{   
    set_offset(pos);
    set_fixed_dim(glm::vec2(content_dim.x, content_dim.y + HEADER_HEIGHT));
    set_should_clip_children(true);

    // header
    header.set_fixed_dim(glm::vec2 {content_dim.x, HEADER_HEIGHT});
    header.set_anchor(glm::vec2(0), glm::vec2(1, 0));
    header.set_on_drag_begin([&]() -> void {
        drag_start_pos = get_pos();
    });
    header.set_on_move([&](glm::vec2 delta) -> void {
        set_offset(drag_start_pos + delta);
    });

    // title
    // vertically center, horizontal right align title text
    title.set_fixed_dim(glm::vec2 {content_dim.x, HEADER_HEIGHT});
    title.set_anchor(glm::vec2(0, 0), glm::vec2(1, 0));
    title.set_margin_min(glm::vec2(5, 0));
    title.set_margin_max(glm::vec2(HEADER_HEIGHT + 5, 0));
    title.do_vertical_centering(true);
    title.do_horizontal_centering(false);
    title.set_should_clip(true);

    // contents
    contents.set_offset(glm::vec2(0));
    contents.set_anchor(glm::vec2(0), glm::vec2(1));
    contents.set_margin_min(glm::vec2(0, HEADER_HEIGHT));

    // edges
    edges.set_anchor(glm::vec2(0), glm::vec2(1));
    edges.set_internal_width(EDGE_INTERNAL_WIDTH);
    edges.set_external_width(EDGE_EXTERNAL_WIDTH);
    edges.set_on_drag_begin([&]() -> void {
        drag_start_pos = get_pos();
        drag_start_dim = get_dim();
    });
    edges.set_on_move([this](float left, float right, float top, float bottom) -> void {
        // (a, b) are outward deltas of the two sides on this axis
        // at most one of (a, b) are non-zero
        auto clamp_axis = [](float& a, float& b, float start_len, float min_len) {
            // if the window already starts below the minimum, just don't let it shrink further
            float min_sum = std::min(min_len, start_len) - start_len;
            float sum = a + b;
            if(sum >= min_sum) {
                // don't have to do any clamping
                return;
            }

            // do clamping
            float deficit = min_sum - sum;
            if(a < 0.0f) a += deficit;
            else b += deficit;
        };
        clamp_axis(left, right,  drag_start_dim.x, WINDOW_MIN_WIDTH);
        clamp_axis(top,  bottom, drag_start_dim.y, WINDOW_MIN_HEIGHT);

        glm::vec2 new_pos = {drag_start_pos.x - left, drag_start_pos.y - top};
        glm::vec2 new_dim = {drag_start_dim.x + left + right, drag_start_dim.y + top + bottom};
        set_offset(new_pos);
        set_fixed_dim(new_dim);
    });

    // close button
    close_button.set_fixed_dim(glm::vec2(HEADER_HEIGHT));
    close_button.set_offset(glm::vec2(0));
    close_button.set_anchor(glm::vec2(1, 0));
    close_button.set_pivot(glm::vec2(1, 0));
    close_button.set_callback(
        [this]() -> void {
            destroy();
        }
    );
}

void Window::update_impl(float dt) {
    // do nothing
}

void Window::render_content(UIRenderer& renderer) {
    // background blur
    if(background_blur) {
        renderer.draw_blur({{0, 0}, get_dim()}, blur_sigma);
    }

    // background tint
    constexpr float ALPHA = 0.7;
    constexpr float COLOR = 0.1;
    renderer.draw_rect({{0, 0}, get_dim()}, {COLOR, COLOR, COLOR, ALPHA});
    
    // header
    renderer.draw_rect({{0, 0}, {get_dim().x, HEADER_HEIGHT}}, HEADER_COLOR);
}

void Window::render_overlay(UIRenderer& renderer) {
    // outline
    renderer.draw_rect_outline(
        {{0, 0}, get_dim()}, 
        is_active() ? ACTIVE_OUTLINE_COLOR : OUTLINE_COLOR,
        -1.0f   // draw 1px outside of window
    );
}

bool Window::hit_test(glm::vec2 pos) {
    return 
        pos.x >= -EDGE_EXTERNAL_WIDTH && pos.y >= -EDGE_EXTERNAL_WIDTH && 
        pos.x <= (get_dim().x + EDGE_EXTERNAL_WIDTH) && pos.y <= (get_dim().y + EDGE_EXTERNAL_WIDTH);
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