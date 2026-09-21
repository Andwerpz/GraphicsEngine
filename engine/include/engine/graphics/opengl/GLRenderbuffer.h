#pragma once
#include <cstdint>
#include <glad/glad.h>

#include "../Texture.h"

namespace engine::graphics::opengl {

class GLGraphicsDevice;

class GLRenderbuffer {
    friend class GLGraphicsDevice;
    using TextureFormat = engine::graphics::TextureFormat;

public:
    GLRenderbuffer(GLRenderbuffer&&);
    ~GLRenderbuffer();

    GLuint get_handle() {
        return handle;
    }

    TextureFormat get_format() {
        return format;
    }

    uint32_t get_width() {
        return width;
    }

    uint32_t get_height() {
        return height;
    }

private:
    GLRenderbuffer(
        TextureFormat format,
        uint32_t width,
        uint32_t height,
        GLGraphicsDevice& device, 
        GLuint handle
    );

    GLRenderbuffer() = delete;
    GLRenderbuffer(const GLRenderbuffer&) = delete;
    GLRenderbuffer& operator=(const GLRenderbuffer&) = delete;
    GLRenderbuffer& operator=(GLRenderbuffer&&) = delete;

    // the device that created this framebuffer
    GLGraphicsDevice& device;

    // OpenGL resource handle
    GLuint handle;

    // format of each pixel 
    TextureFormat format;

    // dimensions of renderbuffer
    uint32_t width;
    uint32_t height;
};

}