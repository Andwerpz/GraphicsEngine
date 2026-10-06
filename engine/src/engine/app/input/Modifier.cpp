#include <engine/app/input/Modifier.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace engine::app {

inline Modifier translate_glfw_modifiers(int mods) {
    Modifier result = Modifier::None;
    if(mods & GLFW_MOD_SHIFT)     result = result | Modifier::Shift;
    if(mods & GLFW_MOD_CONTROL)   result = result | Modifier::Ctrl;
    if(mods & GLFW_MOD_ALT)       result = result | Modifier::Alt;
    if(mods & GLFW_MOD_SUPER)     result = result | Modifier::Super;
    if(mods & GLFW_MOD_CAPS_LOCK) result = result | Modifier::CapsLock;
    if(mods & GLFW_MOD_NUM_LOCK)  result = result | Modifier::NumLock;
    return result;
}

}