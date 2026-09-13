#pragma once
#include <cstdint>

namespace engine::graphics::opengl {

// a Buffer represents some contiguous memory resource on the GPU

class GraphicsDevice;

// what sort of thing is this buffer meant to be used for?
enum class BufferUsage {
    Vertex,     // holding geometric primitive information
    Index,      // indexing into other buffers during rendering
    Uniform,    // read-only shader parameters
    Storage,    // general shader readable/writable buffer
};

// memory access capabilities for CPU
enum class BufferMemory : uint64_t {
    Read            = 1 << 0,
    Write           = 1 << 1,
};

class Buffer {
public:
    uint64_t get_size() const {
        return size;
    }

    BufferMemory get_memory() const {
        return memory;
    }

private:
    friend class GraphicsDevice;

    // opaque handle managed by the underlying graphics API
    uint64_t handle;

    // the device that created this buffer
    GraphicsDevice* device;

    // the size in bytes 
    uint64_t size;

    // the sort of memory operations the driver should expect this buffer to be used for
    BufferMemory memory;

    ~Buffer();
};

}

