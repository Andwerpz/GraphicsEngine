#pragma once
#include <cstdint>
#include "Resource.h"

namespace engine::graphics {

// a Buffer represents some contiguous memory on the GPU 

// memory access capabilities for CPU
enum class BufferMemory : uint64_t {
    Read            = 1 << 0,
    Write           = 1 << 1,
};

class Buffer : public Resource {
public:
    Buffer(Buffer&& other);
    virtual ~Buffer() = 0;

protected:
    Buffer(uint64_t size, BufferMemory memory);

    // size of buffer in bytes
    uint64_t size;

    // what is the CPU allowed to do with this buffer after it's created
    BufferMemory memory;

public:
    uint64_t get_size() const {
        return size;
    }

    BufferMemory get_memory() const {
        return memory;
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