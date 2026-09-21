#include <engine/graphics/opengl/GLRenderbuffer.h>
#include <engine/graphics/opengl/GLGraphicsDevice.h>

namespace engine::graphics::opengl {

GLRenderbuffer::GLRenderbuffer(
    TextureFormat format,
    uint32_t width,
    uint32_t height,
    GLGraphicsDevice& device, 
    GLuint handle
) : format(format),
    width(width),
    height(height),
    device(device),
    handle(handle)
{
    // do nothing
}

GLRenderbuffer::GLRenderbuffer(GLRenderbuffer&& other) :
    device(other.device),
    handle(other.handle)
{
    other.handle = 0;
}

GLRenderbuffer::~GLRenderbuffer() {
    if(handle != 0) {
        device.gl_delete_renderbuffer(handle);
    }
}

}