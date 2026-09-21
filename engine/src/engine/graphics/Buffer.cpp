#include <engine/graphics/Buffer.h>

namespace engine::graphics {

Buffer::Buffer(Buffer&& other) {
    size = other.size;
    memory = other.memory;
}

Buffer::Buffer(uint64_t _size, BufferMemory _memory) {
    size = _size;
    memory = _memory;
}

Buffer::~Buffer() = default;

}