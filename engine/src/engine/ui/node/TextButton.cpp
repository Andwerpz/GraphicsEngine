#include <engine/ui/node/TextButton.h>

namespace engine::ui {

TextButton::TextButton(
    std::string text,
    Font& font,
    std::function<void()> function,
    UIRect bounds
) : Button(function, bounds),
    label(emplace_child<Label>(text, font, UIRect {{0, 0}, bounds.dim}))
{
    label.do_vertical_centering(true);
    label.do_horizontal_centering(true);
}

}