#include <engine/graphics/opengl/GLContext.h>
#include <GLFW/glfw3.h>

namespace engine::graphics::opengl {

GLContext::GLContext(GLContext&& other) :
    device(other.device),
    glfw_window(other.glfw_window)
{
    other.glfw_window = nullptr;
}

GLContext::~GLContext() {
    if(glfw_window != nullptr) {
        glfwDestroyWindow(glfw_window);
    }
}

}