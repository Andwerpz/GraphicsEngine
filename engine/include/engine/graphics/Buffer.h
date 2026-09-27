#pragma once
#include <cstdint>
#include "Resource.h"

namespace engine::graphics {

// a Buffer represents some contiguous memory on the GPU 

enum class BufferUsage {
    Vertex,     // holding geometric primitive information
    Index,      // indexing into other buffers during rendering
    Uniform,    // read-only shader parameters
    Storage,    // general shader readable/writable buffer
};

enum class BufferFrequency {
    Static,     // contents remain unchanged after initialization
    Dynamic,    // contents may be changed between executions
};

class Buffer : public Resource {
public:
    Buffer(Buffer&& other);
    virtual ~Buffer() = 0;

protected:
    Buffer(uint64_t size, BufferUsage usage, BufferFrequency frequency);

    // size of buffer in bytes
    uint64_t size;

    // what is this buffer meant to be used for
    BufferUsage usage;

    // how often is this buffer going to be updated from the CPU
    BufferFrequency frequency;

public:
    uint64_t get_size() const {
        return size;
    }

    BufferUsage get_usage() const {
        return usage;
    }

    BufferFrequency get_frequency() const {
        return frequency;
    }

    // replaces [offset, offset + size) in this buffer with data
    virtual void upload_data(
        const void* data,
        uint64_t size,
        uint64_t offset = 0
    ) = 0;

    // copies subarray [src_offset, src_offset + size) from other to 
    // [dst_offset, dst_offset + size) in this buffer
    virtual void copy_data(
        Buffer& other,
        uint64_t size,
        uint64_t src_offset = 0,
        uint64_t dst_offset = 0
    ) = 0;
};

}