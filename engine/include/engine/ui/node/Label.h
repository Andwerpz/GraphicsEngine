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
        UIContext& context,
        std::string text,
        std::string_view font_addr,
        float font_size
    );

    Label(
        UIContext& context,
        std::string text
    );

    std::string& get_text() {
        return text;
    }
    float get_text_width() {
        return text_width;
    }

    void set_text(std::string _text) {
        text = _text;
        text_width = font.measure_text_width(text);
    }
    void set_color(glm::vec4 _color) {
        color = _color;
    }

    void do_vertical_centering(bool val) {
        vertical_centering = val;
    }
    void do_horizontal_centering(bool val) {
        horizontal_centering = val;
    }
    void do_right_alignment(bool val) {
        right_alignment = val;
    }
    void do_bottom_alignment(bool val) {
        bottom_alignment = val;
    }

    // allow others to change clip behaviour
    using UINode::set_should_clip;

private:
    void update_impl(float dt) override;
    void render_content(UIRenderer& renderer) override;

    // labels are supposed to be decorative
    virtual bool hit_testable() const   { return false; }

    // text to be rendered
    std::string text;

    // font to render the text with
    // should be gotten from the owning UIContext
    Font& font;

    // color to render the text with
    glm::vec4 color = glm::vec4(1);

    // precomputed text width in pixels
    float text_width;

    // will center the rendered text vertically/horizontally within the bounds 
    bool vertical_centering = true;
    bool horizontal_centering = true;

    // will align the left/bottom of the text to the left/bottom of the bounds
    bool right_alignment = false;
    bool bottom_alignment = false;

    // TODO implement this. Probably want to figure out on_resize first
    // // if true, will automatically truncate the text in this label with "..." 
    // //   so that it fits within the bounds 
    // // if the bounds are small enough so that "..." does not fit, it may not render it all
    // bool truncate = false;
};

}