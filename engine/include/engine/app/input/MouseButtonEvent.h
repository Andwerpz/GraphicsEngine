#pragma once

#include "Modifier.h"

namespace engine::app {

enum class MouseButton {
    Button1,    // left
    Button2,    // right
    Button3,    // middle
    Button4,
    Button5,
    Button6,
    Button7,
    Button8,

    Unknown
};

enum class MouseButtonAction {
    Press,
    Release
};

struct MouseButtonEvent {
    MouseButton button;
    MouseButtonAction action;
    Modifier modifiers;

    static MouseButtonEvent translate_glfw_mouse_button_event(int button, int action, int mods);
};

}