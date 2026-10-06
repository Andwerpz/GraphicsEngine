#include <engine/app/input/MouseButtonEvent.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cassert>

#include "Modifier.cpp"

namespace engine::app {

MouseButton translate_glfw_mouse_button(int button) {
    switch(button) {
        case GLFW_MOUSE_BUTTON_LEFT:   return MouseButton::Button1;
        case GLFW_MOUSE_BUTTON_RIGHT:  return MouseButton::Button2;
        case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::Button3;
        case GLFW_MOUSE_BUTTON_4:      return MouseButton::Button4;
        case GLFW_MOUSE_BUTTON_5:      return MouseButton::Button5;
        case GLFW_MOUSE_BUTTON_6:      return MouseButton::Button6;
        case GLFW_MOUSE_BUTTON_7:      return MouseButton::Button7;
        case GLFW_MOUSE_BUTTON_8:      return MouseButton::Button8;
        default:                       return MouseButton::Unknown;
    }
}

MouseButtonAction translate_glfw_mouse_button_action(int action) {
    switch(action) {
        case GLFW_PRESS:
            return MouseButtonAction::Press;
        case GLFW_RELEASE:
            return MouseButtonAction::Release;
        default:
            assert(false);
    }
}

MouseButtonEvent MouseButtonEvent::translate_glfw_mouse_button_event(
    int button,
    int action,
    int mods
) {
    return MouseButtonEvent {
        .button = translate_glfw_mouse_button(button),
        .action = translate_glfw_mouse_button_action(action),
        .modifiers = translate_glfw_modifiers(mods)
    };
}

}