#pragma once
#include <optional>

#include "../RenderTarget.h"
#include "GLFramebuffer.h"
#include "GLRenderbuffer.h"
#include "GLWindow.h"

namespace engine::graphics::opengl {

// a GLTextureRenderTarget is pretty much just a wrapper over GLFramebuffer that manages its own depth buffer
// a GLWindowRenderTarget is just a wrapper over a GLWindow

class GLGraphicsDevice;
class GLTexture;
class GLRenderbuffer;
class GLFramebuffer;
class GLWindow;

class GLTextureRenderTarget : public engine::graphics::TextureRenderTarget {
    friend class GLGraphicsDevice;

public:
    GLTextureRenderTarget(GLTextureRenderTarget&&) = default;
    ~GLTextureRenderTarget() = default;

    GLFramebuffer& get_framebuffer() {
        return gl_framebuffer;
    }

private:
    GLTextureRenderTarget(
        uint32_t width, 
        uint32_t height,
        GLGraphicsDevice& device,
        GLFramebuffer gl_framebuffer
    );

    GLTextureRenderTarget() = delete;
    GLTextureRenderTarget(const GLTextureRenderTarget&) = delete;
    GLTextureRenderTarget& operator=(const GLTextureRenderTarget&) = delete;
    GLTextureRenderTarget& operator=(GLTextureRenderTarget&&) = delete;

    GLGraphicsDevice& device;

    // internally managed framebuffer
    GLFramebuffer gl_framebuffer;

    // internally managed depth buffer
    std::optional<GLRenderbuffer> gl_depth_buffer;

    void bind_texture_impl(uint32_t location, Texture& tex) override;
    void bind_depth_buffer_impl() override;
    void validate_impl() override;
};

class GLWindowRenderTarget : public engine::graphics::WindowRenderTarget {
    friend class GLGraphicsDevice;

public:
    GLWindowRenderTarget(GLWindowRenderTarget&&) = default;
    ~GLWindowRenderTarget() = default;

    GLFramebuffer& get_framebuffer() {
        return gl_framebuffer;
    }

private:
    GLWindowRenderTarget(
        GLGraphicsDevice& device
    );

    GLWindowRenderTarget() = delete;
    GLWindowRenderTarget(const GLWindowRenderTarget&) = delete;
    GLWindowRenderTarget& operator=(const GLWindowRenderTarget&) = delete;
    GLWindowRenderTarget& operator=(GLWindowRenderTarget&&) = delete;
    
    GLGraphicsDevice& device;

    // underlying window
    GLWindow& gl_window;

    // internally managed framebuffer with handle = 0
    GLFramebuffer gl_framebuffer;

    bool has_depth_buffer() override;
    bool has_output(uint32_t location) override;
    void validate_impl() override;
};

}