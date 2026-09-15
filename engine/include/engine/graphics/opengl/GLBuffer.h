#pragma once
#include <cstdint>
#include <glad/glad.h>

namespace engine::graphics::opengl {

// a GLBuffer represents some contiguous memory resource on the GPU

class OpenGLGraphicsDevice;

// what sort of thing is this buffer meant to be used for?
enum class GLBufferUsage {
    Vertex,     // holding geometric primitive information
    Index,      // indexing into other buffers during rendering
    Uniform,    // read-only shader parameters
    Storage,    // general shader readable/writable buffer
};

// memory access capabilities for CPU
enum class GLBufferMemory : uint64_t {
    Read            = 1 << 0,
    Write           = 1 << 1,
};

class GLBuffer {
public:
    GLBuffer(GLBuffer&&);
    ~GLBuffer();

    uint64_t get_size() const {
        return size;
    }

    GLBufferMemory get_memory() const {
        return memory;
    }

private:
    friend class OpenGLGraphicsDevice;

    GLBuffer(OpenGLGraphicsDevice& device, GLuint handle, uint64_t size, GLBufferMemory memory) :
        device(device), handle(handle), size(size), memory(memory) {}

    GLBuffer() = delete;
    GLBuffer(const GLBuffer&) = delete;
    GLBuffer& operator=(const GLBuffer&) = delete;
    GLBuffer& operator=(GLBuffer&&) = delete;

    // the device that created this buffer
    OpenGLGraphicsDevice& device;

    // OpenGL resource handle
    GLuint handle;

    // the size in bytes 
    uint64_t size;

    // the sort of memory operations the driver should expect this buffer to be used for
    GLBufferMemory memory;
};

}

