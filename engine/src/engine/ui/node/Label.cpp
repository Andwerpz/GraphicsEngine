#include <engine/ui/node/Label.h>
#include <iostream>

namespace engine::ui {

Label::Label(
    std::string text,
    Font& font,
    UIRect bounds
) : UINode(bounds),
    text(text),
    font(font)
{
    text_width = font.measure_text_width(text);
}

void Label::update_impl(float dt) {
    // do nothing
}

void Label::render_impl(UIRenderer& renderer) {
    // by default, left and top align the text
    glm::vec2 pen = bounds.pos;
    pen.y += font.get_ascent();
    glm::vec2 center = bounds.pos + bounds.dim / 2.0f;

    // vertical alignment
    if(vertical_centering) {
        pen.y = center.y + (font.get_ascent() - font.get_descent()) / 2.0f;
    }

    // horizontal alignment
    if(horizontal_centering) {
        pen.x = center.x - text_width / 2.0f;
    }

    // draw command
    renderer.draw_text(text, pen, font);

    // font debug lines
    if(true) {
        float ascent  = font.get_ascent();
        float descent = font.get_descent();

        glm::vec2 left  = {pen.x, pen.y};
        glm::vec2 right = {pen.x + text_width, pen.y};

        // baseline (red)
        renderer.draw_line(left, right, 1, {1.0, 0.0, 0.0, 1.0});

        // ascent line (green), above the baseline
        glm::vec2 a0 = left,  a1 = right;
        a0.y -= ascent;  a1.y -= ascent;
        renderer.draw_line(a0, a1, 1, {0.0, 1.0, 0.0, 1.0});

        // descent line (blue), below the baseline
        glm::vec2 d0 = left,  d1 = right;
        d0.y += descent;  d1.y += descent;
        renderer.draw_line(d0, d1, 1, {0.3, 0.3, 1.0, 1.0});
    }
}

}