#pragma once
#include <glad/glad.h>

class GLFWwindow;

namespace engine::graphics::opengl {

class GLGraphicsDevice;

class GLContext {
    friend class GLGraphicsDevice;

public:
    GLContext(GLContext&&);
    ~GLContext();

    GLFWwindow* get_glfw_window() {
        return glfw_window;
    }
    
    bool operator==(const GLContext& other) const {
        return (&device == &other.device) && (glfw_window == other.glfw_window);
    }

    bool operator!=(const GLContext& other) const {
        return !(*this == other);
    }

private:
    GLContext(
        GLGraphicsDevice& device,
        GLFWwindow* glfw_window
    ) : device(device), glfw_window(glfw_window) {}

    GLContext() = delete;
    GLContext(const GLContext&) = delete;
    GLContext& operator=(const GLContext&) = delete;
    GLContext& operator=(GLContext&&) = delete;

    // the device that created this context
    GLGraphicsDevice& device;

    // underlying GLFW window
    GLFWwindow* glfw_window;
};

}