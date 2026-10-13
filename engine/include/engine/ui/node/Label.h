#pragma once
#include <engine/ui/UINode.h>
#include <engine/asset/Font.h>

#include <iostream>

namespace engine::ui {

// a Label just owns a piece of text and knows how to render it nicely

class Label : public UINode {
    using Font = engine::asset::Font;

public:
    Label(
        std::string text,
        Font& font,
        UIRect bounds
    );

    std::string& get_text() {
        return text;
    }

    void set_text(std::string _text) {
        text = _text;
        text_width = font.measure_text_width(text);
    }

    void do_vertical_centering(bool val) {
        vertical_centering = val;
    }

    void do_horizontal_centering(bool val) {
        horizontal_centering = val;
    }

private:
    void update_impl(float dt) override;
    void render_impl(UIRenderer& renderer) override;

    // text to be rendered
    std::string text;

    // font to render the text with
    Font& font;

    // precomputed text width in pixels
    float text_width;

    // will center the rendered text vertically/horizontally within the bounds 
    bool vertical_centering = true;
    bool horizontal_centering = true;

    void on_mouse_enter_impl() override {
        std::cout << "MOUSE ENTER" << std::endl;
    }

    void on_mouse_leave_impl() override {
        std::cout << "MOUSE LEAVE" << std::endl;
    }
};

}