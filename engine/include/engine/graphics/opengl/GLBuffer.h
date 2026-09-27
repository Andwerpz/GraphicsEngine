#pragma once
#include <cstdint>
#include <glad/glad.h>

#include "../Buffer.h"

namespace engine::graphics::opengl {

class GLGraphicsDevice;

class GLBuffer : public engine::graphics::Buffer {
    friend class GLGraphicsDevice;

public:
    GLBuffer(GLBuffer&&);
    ~GLBuffer() override;

    GLuint get_handle() {
        return handle;
    }

    void upload_data(
        const void* data,
        uint64_t size,
        uint64_t offset = 0
    ) override;

    void copy_data(
        Buffer& other,
        uint64_t size,
        uint64_t src_offset,
        uint64_t dst_offset
    ) override;

private:
    GLBuffer(
        uint64_t size, 
        BufferUsage usage,
        BufferFrequency frequency,
        GLGraphicsDevice& device, 
        GLuint handle
    );

    GLBuffer() = delete;
    GLBuffer(const GLBuffer&) = delete;
    GLBuffer& operator=(const GLBuffer&) = delete;
    GLBuffer& operator=(GLBuffer&&) = delete;

    // the device that created this buffer
    GLGraphicsDevice& device;

    // OpenGL resource handle
    GLuint handle;
};

}

