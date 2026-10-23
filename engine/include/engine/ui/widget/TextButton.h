#pragma once
#include <engine/ui/node/Label.h>
#include <engine/ui/node/Button.h>
#include <engine/ui/NodeStateColor.h>

namespace engine::ui {

// a Button with a responsive background and a centered label

class TextButton : public Button {
    using Font = engine::asset::Font;

public:
    TextButton(
        UIContext& context,
        std::function<void()> callback,
        std::string text,
        std::string_view font_addr = "engine://fonts/consolas.ttf",
        float font_size = 12.0f
    ) : Button(context),
        label(emplace_child<Label>(text, font_addr, font_size)),
        color(*this)
    {
        set_callback(callback);

        label.set_anchor(glm::vec2(0), glm::vec2(1));
        label.do_vertical_centering(true);
        label.do_horizontal_centering(true);
    }

private:
    void render_content(UIRenderer& renderer) override {
        // background rect
        renderer.draw_rect({{0, 0}, get_dim()}, color.get_color());
    }

    Label& label;
    NodeStateColor color;
};

}