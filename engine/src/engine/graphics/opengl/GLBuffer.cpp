#include <engine/graphics/opengl/GLBuffer.h>
#include <engine/graphics/opengl/GLGraphicsDevice.h>
#include <engine/util/error.h>

#include <iostream>

namespace engine::graphics::opengl {

GLBuffer::GLBuffer(GLBuffer&& other) : 
    engine::graphics::Buffer(std::move(other)),
    device(other.device), 
    handle(other.handle) {
    other.handle = 0;
}

GLBuffer::~GLBuffer() {
    if(handle != 0) {
        device.gl_delete_buffer(*this);
    }
}

GLBuffer::GLBuffer(
    uint64_t size, 
    BufferUsage usage,
    BufferFrequency frequency,
    GLGraphicsDevice& device, 
    GLuint handle
) : engine::graphics::Buffer(size, usage, frequency),
    device(device),
    handle(handle)
{
    // do nothing
}

void GLBuffer::upload_data(
    const void* data,
    uint64_t size,
    uint64_t offset
) {
    if(get_frequency() != BufferFrequency::Dynamic) {
        engine::util::throw_error("can only upload_data to a buffer with dynamic frequency");
    }
    if(data == nullptr) {
        engine::util::throw_error("data cannot be nullptr");
    }
    if(size + offset > get_size()) {
        engine::util::throw_error("upload range out of bounds");
    }
    glNamedBufferSubData(handle, offset, size, data);
}

void GLBuffer::copy_data(
    Buffer& _other,
    uint64_t size,
    uint64_t src_offset,
    uint64_t dst_offset
) {
    assert(dynamic_cast<engine::graphics::opengl::GLBuffer*>(&_other) != nullptr);
    GLBuffer& other = dynamic_cast<engine::graphics::opengl::GLBuffer&>(_other);
    assert(&other.device == &device);
    if(size + src_offset > get_size()) {
        engine::util::throw_error("copy goes out of range of this buffer");
    }
    if(size + dst_offset > other.get_size()) {
        engine::util::throw_error("copy goes out of range of other buffer");
    }
    
    GLuint gl_src_handle = other.handle;
    GLuint gl_dst_handle = handle;
    glCopyNamedBufferSubData(
        gl_src_handle,
        gl_dst_handle,
        src_offset,
        dst_offset,
        size
    );
}

}

