#pragma once
#include <cstdint>
#include <glad/glad.h>

namespace engine::graphics::opengl {

// a GLFramebuffer just represents something that can be rendered to 
// the underlying resource being rendered to is implementation specific 

class OpenGLGraphicsDevice;

class GLFramebuffer {
public:
    GLFramebuffer(GLFramebuffer&&);
    ~GLFramebuffer();
  
private:
    friend class OpenGLGraphicsDevice;

    GLFramebuffer(OpenGLGraphicsDevice& device, GLuint handle) :
        device(device), handle(handle) {}

    GLFramebuffer() = delete;
    GLFramebuffer(const GLFramebuffer&) = delete;
    GLFramebuffer& operator=(const GLFramebuffer&) = delete;
    GLFramebuffer& operator=(GLFramebuffer&&) = delete;

    // the device that created this framebuffer
    OpenGLGraphicsDevice& device;

    // OpenGL resource handle
    GLuint handle;
};

}