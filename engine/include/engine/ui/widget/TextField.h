#pragma once
#include <engine/ui/UINode.h>
#include <engine/ui/node/Label.h>
#include <engine/ui/NodeStateColor.h>

namespace engine::ui {

// a TextField knows how to translate key events into text input from the user
// TODO separate the input receiving part out into node/TextInput

class TextField : public UINode {
    static inline constexpr glm::vec4 HINT_TEXT_COLOR = {0.6, 0.6, 0.6, 0.5};
    static inline constexpr float TEXT_MARGIN = 3.0f;

public:
    TextField(
        UIContext& context
    ) : UINode(context),
        text(emplace_child<Label>("", "engine://res/fonts/consolas.ttf", 12.0)),
        color(*this)
    {
        // text
        text.set_anchor(glm::vec2(0), glm::vec2(1));
        text.set_margin_min(glm::vec2(TEXT_MARGIN, 0));
        text.set_margin_max(glm::vec2(TEXT_MARGIN, 0));
        text.set_should_clip(true);
        text.do_horizontal_centering(false);
        text.do_vertical_centering(true);
        update_text();
    }

    std::string get_input_text() {
        return input_text;
    }
    void set_input_text(std::string _text) {
        if(_text == input_text) {
            return;
        }
        input_text = _text;
        update_text();
        if(input_changed_callback) input_changed_callback(input_text);
    }
    void set_hint_text(std::string _text) {
        hint_text = _text;
        update_text();
    }
    void set_input_changed_callback(std::function<void(const std::string&)> callback) {
        input_changed_callback = callback;
    }

private:
    // updates the rendered text Label to match the current state of the text field
    void update_text() {
        if(input_text.size() == 0) {
            // render hint text
            text.set_text(hint_text);
            text.set_color(HINT_TEXT_COLOR);
            text.do_right_alignment(false);
        }
        else {
            // render input text
            text.set_text(input_text);
            text.set_color(glm::vec4(1));
            text.do_right_alignment(text.get_text_width() >= text.get_dim().x);
        }
    }

    void render_content(UIRenderer& renderer) override {
        // background rect
        renderer.draw_rect({{0, 0}, get_dim()}, color.get_color());
    }

    void render_overlay(UIRenderer& renderer) override {
        // cursor
        if(is_focused()) {
            float text_width = input_text.size() == 0 ? 0 : text.get_text_width();
            float cursor_xoff = TEXT_MARGIN + text_width;
            cursor_xoff = std::min(cursor_xoff, get_dim().x - TEXT_MARGIN * 2);
            cursor_xoff += 1;
            float cursor_yoff = TEXT_MARGIN;
            float cursor_width = 2.0f;
            float cursor_height = get_dim().y - TEXT_MARGIN * 2;
            renderer.draw_rect(Bounds {{cursor_xoff, cursor_yoff}, {cursor_width, cursor_height}}, glm::vec4(1));
        }
    }

    bool on_mouse_button(const MouseButtonEvent& ev) override {
        // capture all mouse input
        return true;
    }

    bool on_key(const KeyEvent& ev) override {
        // backspace
        if(ev.key == Key::Backspace && (ev.action == KeyAction::Press || ev.action == KeyAction::Repeat)) {
            if(input_text.size() != 0) {
                set_input_text(input_text.substr(0, input_text.size() - 1));
            }
            return true;
        }

        // append char
        std::optional<char> c = ev.to_char();
        if(c.has_value()) {
            set_input_text(input_text + c.value());
            return true;
        }
        return false;
    }

    // needs to be focusable to get keyboard input
    bool focusable() const override { return true; }

    // text rendered when there is no input text 
    std::string hint_text = "write something";

    // text the user has written so far
    std::string input_text;

    // rendered text
    Label& text;

    // callback triggered whenever a key event arrives that causes the input to change
    // the input text is passed into the callback
    std::function<void(const std::string&)> input_changed_callback;

    // background color
    NodeStateColor color;
};

}