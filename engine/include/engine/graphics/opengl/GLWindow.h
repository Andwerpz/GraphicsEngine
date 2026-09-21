#pragma once
#include <cassert>
#include <cstdint>

#include "../Window.h"

struct GLFWwindow;

namespace engine::graphics::opengl {

// represents an GLFW OS window 

class GLWindow : public engine::graphics::Window {
public:
    GLWindow(GLFWwindow* glfw_window);
    GLWindow(GLWindow&& other);
    ~GLWindow();

    uint32_t get_width() override;
    uint32_t get_height() override;

    void swap_buffers();

private:
    GLFWwindow* glfw_window;
};

}
