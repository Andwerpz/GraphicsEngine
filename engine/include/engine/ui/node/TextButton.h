#pragma once
#include <engine/ui/node/Label.h>
#include <engine/ui/node/Button.h>

namespace engine::ui {

// a button with a centered label

class TextButton : public Button {
    using Font = engine::asset::Font;

public:
    TextButton(
        std::string text,
        Font& font,
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