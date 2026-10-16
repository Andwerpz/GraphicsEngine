#include <engine/ui/node/UIRoot.h>
#include <engine/ui/UIContext.h>

namespace engine::ui {

glm::vec2 UIRoot::get_absolute_mouse_pos() {
    return context.get_mouse_pos();
}

}