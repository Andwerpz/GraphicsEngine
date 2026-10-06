#pragma once
#include <cstdint>
#include <glad/glad.h>
#include <span>

namespace engine::graphics::opengl {

class GLGraphicsDevice;
class GLContext;
class GLTexture;
class GLRenderbuffer;

class GLFramebuffer {
    friend class GLGraphicsDevice;

public:
    GLFramebuffer(GLFramebuffer&&);
    ~GLFramebuffer();

    GLuint get_handle() {
        return handle;
    }

    GLContext& get_context() {
        return context;
    }

    // binds a texture as the color attachment 'GL_COLOR_ATTACHMENT0 + index'
    void gl_bind_color_attachment(
        uint32_t index,
        GLTexture& texture
    );

    // binds a renderbuffer as the depth attachment 'GL_DEPTH_ATTACHMENT'
    void gl_bind_depth_attachment(
        GLRenderbuffer& renderbuffer
    );

    // establishes the fragment shader output -> framebuffer color attachment mapping
    // for example, if we had 'bufs = {GL_COLOR_ATTACHMENT2, GL_NONE, GL_COLOR_ATTACHMENT1}'
    // then that would represent the mapping 
    //   output0 -> GL_COLOR_ATTACHMENT2
    //   output1 -> discarded
    //   output2 -> GL_COLOR_ATTACHMENT1
    void gl_set_draw_buffers(std::initializer_list<GLenum> bufs);
    void gl_set_draw_buffers(std::span<GLenum> bufs);

    // returns true if is complete, false otherwise
    bool gl_is_complete();
  
private:
    GLFramebuffer(
        GLGraphicsDevice& device, 
        GLContext& context, 
        GLuint handle
    ) : device(device), context(context), handle(handle) {}

    GLFramebuffer() = delete;
    GLFramebuffer(const GLFramebuffer&) = delete;
    GLFramebuffer& operator=(const GLFramebuffer&) = delete;
    GLFramebuffer& operator=(GLFramebuffer&&) = delete;

    // the device that created this framebuffer
    GLGraphicsDevice& device;

    // the opengl context this framebuffer belongs to
    GLContext& context;

    // OpenGL resource handle
    GLuint handle;
};

}