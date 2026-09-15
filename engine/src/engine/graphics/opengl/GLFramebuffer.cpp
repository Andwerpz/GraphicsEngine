#include <engine/graphics/opengl/GLFramebuffer.h>
#include <engine/graphics/opengl/OpenGLGraphicsDevice.h>

namespace engine::graphics::opengl {

GLFramebuffer::GLFramebuffer(GLFramebuffer&& other) :
device(other.device), handle(other.handle) {
    other.handle = 0;
}

GLFramebuffer::~GLFramebuffer() {
    if(handle != 0) {
        device.gl_delete_framebuffer(handle);
    }
}

}

