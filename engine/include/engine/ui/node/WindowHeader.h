#pragma once
#include <engine/ui/node/DragHandle.h>
#include <engine/ui/node/Label.h>

namespace engine::ui {

class WindowHeader : public DragHandle {
public:
    WindowHeader(
        UIContext& context,
        std::string title_text,
        UIRect bounds
    );  

private:
    // title rendered on the left side
    Label& title;

    // TODO close button on right
};

}