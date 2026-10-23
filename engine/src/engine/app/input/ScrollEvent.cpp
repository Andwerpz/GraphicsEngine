#include <engine/app/input/ScrollEvent.h>
#include "Modifier.cpp"

namespace engine::app {

ScrollEvent ScrollEvent::translate_glfw_scroll_event(double xoff, double yoff, int mods) {
    return ScrollEvent {
        .xoff = xoff,
        .yoff = yoff,
        .modifier = translate_glfw_modifiers(mods)
    };
}

}