#include <engine/ui/node/TextButton.h>

namespace engine::ui {

TextButton::TextButton(
    UIContext& context,
    std::string text,
    std::string_view font_addr,
    float font_size,
    std::function<void()> function
) : Button(context),
    label(emplace_child<Label>(text, font_addr, font_size))
{
    label.set_anchor(glm::vec2(0), glm::vec2(1));
    label.do_vertical_centering(true);
    label.do_horizontal_centering(true);

    set_callback(function);
}

}