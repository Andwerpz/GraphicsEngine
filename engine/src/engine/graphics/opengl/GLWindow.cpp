#include <engine/graphics/opengl/GLWindow.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace engine::graphics::opengl {

GLWindow::GLWindow(GLWindow&& other) : 
    device(other.device),  
    context(std::move(other.context))
{
    // do nothing
}

uint32_t GLWindow::get_width() {
    int width, height;
    glfwGetFramebufferSize(context.get_glfw_window(), &width, &height);
    assert(width != 0);
    return width;
}

uint32_t GLWindow::get_height() {
    int width, height;
    glfwGetFramebufferSize(context.get_glfw_window(), &width, &height);
    assert(height != 0);
    return height;
}

void GLWindow::swap_buffers() {
    std::cout << "Swapping buffers" << std::endl;
    glfwSwapBuffers(context.get_glfw_window());

    GLint double_buffered = GL_FALSE;
    glGetIntegerv(GL_DOUBLEBUFFER, &double_buffered);
    std::cout << "Double buffered: " << (double_buffered == GL_TRUE ? "yes" : "no") << '\n';
}

}
