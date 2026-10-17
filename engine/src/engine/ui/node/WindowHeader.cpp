#include <engine/ui/node/WindowHeader.h>

namespace engine::ui {

WindowHeader::WindowHeader(
    UIContext& context,
    std::string title_text,
    UIRect bounds
) : DragHandle(context, bounds),
    title(emplace_child<Label>(
        title_text,
        "engine://res/fonts/consolas.ttf",
        12.0,
        UIRect {{5, 0}, {0, bounds.dim.y}}  
    ))
{
    // vertically center, horizontal right align text
    title.do_vertical_centering(true);
    title.do_horizontal_centering(false);
}

}