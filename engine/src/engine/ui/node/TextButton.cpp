#include <engine/ui/node/TextButton.h>

namespace engine::ui {

TextButton::TextButton(
    UIContext& context,
    std::string text,
    std::string_view font_addr,
    float font_size,
    std::function<void()> function,
    UIRect bounds
) : Button(context, function, bounds),
    label(emplace_child<Label>(text, font_addr, font_size, UIRect {{0, 0}, bounds.dim}))
{
    label.do_vertical_centering(true);
    label.do_horizontal_centering(true);
}

}