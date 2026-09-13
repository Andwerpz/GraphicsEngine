#include <engine/graphics/opengl/Window.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace engine::graphics::opengl {

Window::Window(GLFWwindow* _glfw_window) {
    glfw_window = _glfw_window;
    assert(glfw_window != nullptr);
}

Window::Window(Window&& other) {
    glfw_window = other.glfw_window;
    other.glfw_window = nullptr;
}

Window::~Window() {
    // see if this window has been moved
    if(glfw_window == nullptr) {
        return;
    }
    
    std::cout << "Destroy window" << std::endl;
    assert(glfw_window != nullptr);
    glfwDestroyWindow(glfw_window);
}

uint32_t Window::get_width() {
    int width, height;
    glfwGetFramebufferSize(glfw_window, &width, &height);
    assert(width != 0);
    return width;
}

uint32_t Window::get_height() {
    int width, height;
    glfwGetFramebufferSize(glfw_window, &width, &height);
    assert(height != 0);
    return height;
}

}
