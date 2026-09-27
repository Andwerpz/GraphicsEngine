#include <engine/graphics/Buffer.h>

namespace engine::graphics {

Buffer::Buffer(Buffer&& other) {
    size = other.size;
    usage = other.usage;
    frequency = other.frequency;
}

Buffer::Buffer(uint64_t _size, BufferUsage _usage, BufferFrequency _frequency) {
    size = _size;
    usage = _usage;
    frequency = _frequency;
}

Buffer::~Buffer() = default;

}