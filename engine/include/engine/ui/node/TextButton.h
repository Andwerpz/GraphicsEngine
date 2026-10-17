#pragma once
#include <engine/ui/node/Label.h>
#include <engine/ui/node/Button.h>

namespace engine::ui {

// a button with a centered label

class TextButton : public Button {
    using Font = engine::asset::Font;

public:
    TextButton(
        UIContext& context,
        std::string text,
        std::string_view font_addr,
        float font_size,
        std::function<void()> function,
        UIRect bounds
    );

    Label& get_label() {
        return label;
    }

private:
    Label& label;
};

}