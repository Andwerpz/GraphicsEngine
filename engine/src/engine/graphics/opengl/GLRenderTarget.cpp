#include <engine/graphics/opengl/GLRenderTarget.h>
#include <engine/graphics/opengl/GLTexture.h>
#include <engine/graphics/opengl/GLWindow.h>
#include <engine/graphics/opengl/GLGraphicsDevice.h>
#include <engine/utils/error.h>

namespace engine::graphics::opengl {

GLTextureRenderTarget::GLTextureRenderTarget(
    uint32_t width, 
    uint32_t height,
    GLGraphicsDevice& device,
    GLFramebuffer gl_framebuffer
) : engine::graphics::TextureRenderTarget(width, height),
    device(device),
    gl_framebuffer(std::move(gl_framebuffer))
{
    // do nothing
}

void GLTextureRenderTarget::bind_texture_impl(
    uint32_t location,
    Texture& tex
) {
    if(dynamic_cast<GLTexture*>(&tex) == nullptr) {
        engine::utils::throw_error("trying to bind non-opengl texture to opengl render target");
    }
    GLTexture& gl_tex = dynamic_cast<GLTexture&>(tex);
    gl_framebuffer.gl_bind_color_attachment(location, gl_tex);
}

void GLTextureRenderTarget::bind_depth_buffer_impl() {
    // create new depth renderbuffer
    assert(!gl_depth_buffer.has_value());
    gl_depth_buffer.emplace(device.gl_create_renderbuffer(TextureFormat::Depth32F, width, height));

    // bind to framebuffer
    gl_framebuffer.gl_bind_depth_attachment(gl_depth_buffer.value());
}

void GLTextureRenderTarget::validate_impl() {
    // validate framebuffer
    if(!gl_framebuffer.gl_is_complete()) {
        engine::utils::throw_error("failed to validate framebuffer");
    }
}

void GLTextureRenderTarget::clear(ClearFlags flags) {
    if(flags == ClearFlags::None) return;
    GLenum gl_clear_flags = 
        ((flags & ClearFlags::Color) != ClearFlags::None ? GL_COLOR_BUFFER_BIT : 0) |
        ((flags & ClearFlags::Depth) != ClearFlags::None ? GL_DEPTH_BUFFER_BIT : 0)
    ;

    device.gl_bind_framebuffer(gl_framebuffer);
    glClear(gl_clear_flags);
}


GLWindowRenderTarget::GLWindowRenderTarget(
    GLGraphicsDevice& device,
    GLWindow& window
) : engine::graphics::WindowRenderTarget(window),
    device(device),
    gl_window(window),
    gl_framebuffer(device.gl_create_window_framebuffer(window))
{
    // do nothing
}

bool GLWindowRenderTarget::has_depth_buffer() {
    // GLFW windows should always have a depth buffer (?)
    return true;
}

bool GLWindowRenderTarget::has_output(uint32_t location) {
    // color output to GLFW window is location 0 (?)
    return location == 0;
}

void GLWindowRenderTarget::validate_impl() {
    // do nothing
}

void GLWindowRenderTarget::clear(ClearFlags flags) {
    if(flags == ClearFlags::None) return;
    GLenum gl_clear_flags = 
        ((flags & ClearFlags::Color) != ClearFlags::None ? GL_COLOR_BUFFER_BIT : 0) |
        ((flags & ClearFlags::Depth) != ClearFlags::None ? GL_DEPTH_BUFFER_BIT : 0)
    ;

    device.gl_bind_framebuffer(gl_framebuffer);
    glClear(gl_clear_flags);
}

}