#pragma once
#include <cassert>
#include <cstdint>

struct GLFWwindow;

namespace engine::graphics::opengl {

// represents an GLFW OS window 

class Window {
public:
    Window(GLFWwindow* glfw_window);
    Window(Window&& other);
    ~Window();

    uint32_t get_width();
    uint32_t get_height();

private:
    GLFWwindow* glfw_window;
};

}
