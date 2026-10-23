#pragma once

#include "Modifier.h"

namespace engine::app {

struct ScrollEvent {
    double xoff;    // positive indicates scrolling right
    double yoff;    // positive indicates scrolling up
    Modifier modifier;

    static ScrollEvent translate_glfw_scroll_event(double xoff, double yoff, int mods);
};

}