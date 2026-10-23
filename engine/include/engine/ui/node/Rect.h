#pragma once
#include <engine/ui/node/Container.h>

namespace engine::ui {

// a Rect just knows how to render a rectangle

class Rect : public Container {
public:
    Rect(UIContext& context) : Container(context) {}

    void set_background_color(glm::vec4 val) {
        background_color = val;
    }
    void set_outline_color(glm::vec4 val) {
        outline_color = val;
    }
    void set_outline_inset(float val) {
        outline_inset = val;
    }
    void do_draw_background(bool val) {
        draw_background = val;
    }
    void do_draw_outline(bool val) {
        draw_outline = val;
    }

private:
    void render_content(UIRenderer& renderer) override {
        // background
        if(draw_background) {
            renderer.draw_rect({get_pos(), get_dim()}, background_color);
        }

        // outline
        if(draw_outline) {
            renderer.draw_rect_outline({get_pos(), get_dim()}, outline_color, outline_inset);
        }
    }

    glm::vec4 background_color = {0, 0, 0, 0};
    bool draw_background = false;

    glm::vec4 outline_color = {0, 0, 0, 0};
    float outline_inset = 0.0f;
    bool draw_outline = false;
};

}