#include <engine/graphics/opengl/GLFramebuffer.h>
#include <engine/graphics/opengl/GLGraphicsDevice.h>
#include <engine/graphics/opengl/GLTexture.h>
#include <engine/graphics/opengl/GLRenderbuffer.h>

namespace engine::graphics::opengl {

GLFramebuffer::GLFramebuffer(GLFramebuffer&& other) :
    device(other.device), 
    context(other.context), 
    handle(other.handle) 
{
    other.handle = 0;
}

GLFramebuffer::~GLFramebuffer() {
    if(handle != 0) {
        device.gl_delete_framebuffer(handle);
    }
}

void GLFramebuffer::gl_bind_color_attachment(
    uint32_t index,
    GLTexture& texture
) {
    glNamedFramebufferTexture(
        handle,
        GL_COLOR_ATTACHMENT0 + index,
        texture.get_handle(),
        0
    );
}

void GLFramebuffer::gl_bind_depth_attachment(
    GLRenderbuffer& renderbuffer
) {
    glNamedFramebufferRenderbuffer(
        handle,
        GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER,
        renderbuffer.get_handle()
    );
}

void GLFramebuffer::gl_set_draw_buffers(
    std::initializer_list<GLenum> bufs
) {
    glNamedFramebufferDrawBuffers(
        handle,
        bufs.size(),
        bufs.begin()
    );
}

void GLFramebuffer::gl_set_draw_buffers(
    std::span<GLenum> bufs
) {
    glNamedFramebufferDrawBuffers(
        handle,
        bufs.size(),
        bufs.data()
    );
}

bool GLFramebuffer::gl_is_complete() {
    GLenum status = glCheckNamedFramebufferStatus(
        handle,
        GL_FRAMEBUFFER
    );
    return status == GL_FRAMEBUFFER_COMPLETE;
}

}

