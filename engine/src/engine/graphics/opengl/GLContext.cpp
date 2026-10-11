#include <engine/graphics/opengl/GLContext.h>
#include <engine/graphics/opengl/GLGraphicsDevice.h>
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
        device.gl_delete_context(*this);
    }
}

}