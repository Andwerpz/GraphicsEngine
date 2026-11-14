#include <engine/graphics/opengl/GLRenderTarget.h>
#include <engine/graphics/opengl/GLTexture.h>
#include <engine/graphics/opengl/GLWindow.h>
#include <engine/graphics/opengl/GLGraphicsDevice.h>
#include <engine/util/error.h>

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
        engine::util::throw_error("trying to bind non-opengl texture to opengl render target");
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
        engine::util::throw_error("failed to validate framebuffer");
    }
}

void GLTextureRenderTarget::clear(ClearFlags flags) {
    if(flags == ClearFlags::None) return;
    GLenum gl_clear_flags = 
        ((flags & ClearFlags::Color) != ClearFlags::None ? GL_COLOR_BUFFER_BIT : 0) |
        ((flags & ClearFlags::Depth) != ClearFlags::None ? GL_DEPTH_BUFFER_BIT : 0)
    ;

    device.gl_bind_framebuffer(gl_framebuffer);
    glDisable(GL_SCISSOR_TEST);     // scissor affects clear region
    glClear(gl_clear_flags);
}

std::unique_ptr<RenderTargetView> GLTextureRenderTarget::create_view(Bounds bounds) {
    return std::unique_ptr<RenderTargetView>(new GLRenderTargetView(*this, device, bounds));
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
    glDisable(GL_SCISSOR_TEST);     // scissor affects clear region
    glClear(gl_clear_flags);
}

std::unique_ptr<RenderTargetView> GLWindowRenderTarget::create_view(Bounds bounds) {
    return std::unique_ptr<RenderTargetView>(new GLRenderTargetView(*this, device, bounds));
}


GLRenderTargetView::GLRenderTargetView(
    RenderTarget& target,
    GLGraphicsDevice& device,
    Bounds bounds
) : RenderTargetView(target, bounds),
    device(device)
{
    // do nothing
}

void GLRenderTargetView::clear(ClearFlags flags) {
    if(flags == ClearFlags::None) return;
    GLenum gl_clear_flags = 
        ((flags & ClearFlags::Color) != ClearFlags::None ? GL_COLOR_BUFFER_BIT : 0) |
        ((flags & ClearFlags::Depth) != ClearFlags::None ? GL_DEPTH_BUFFER_BIT : 0)
    ;
    if(bounds.is_empty()) return;

    GLFramebuffer* gl_framebuffer = nullptr;
    if(auto target = dynamic_cast<GLTextureRenderTarget*>(&root_target)) {
        gl_framebuffer = &target->gl_framebuffer;
    }
    else if(auto target = dynamic_cast<GLWindowRenderTarget*>(&root_target)) {
        gl_framebuffer = &target->gl_framebuffer;
    }
    else assert(false);
    assert(gl_framebuffer != nullptr);

    // clear with scissor enabled
    device.gl_bind_framebuffer(*gl_framebuffer);
    device.gl_set_scissor(
        gl_framebuffer->get_context(), 
        bounds.pos.x, bounds.pos.y,
        bounds.dim.x, bounds.dim.y
    );
    glEnable(GL_SCISSOR_TEST);
    glClear(gl_clear_flags);
}

std::unique_ptr<RenderTargetView> GLRenderTargetView::create_view(Bounds bounds) {
    return std::unique_ptr<RenderTargetView>(new GLRenderTargetView(*this, device, bounds));
}

}