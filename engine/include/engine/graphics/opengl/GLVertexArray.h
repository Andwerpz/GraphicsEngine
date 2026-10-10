#pragma once
#include <glad/glad.h>
#include <cassert>
#include <optional>

#include "../ProgramInvocation.h"

namespace engine::graphics::opengl {

class GLGraphicsDevice;
class GLContext;
class GLBuffer;

class GLVertexArray {
    friend class GLGraphicsDevice;
    using IndexFormat = engine::graphics::IndexFormat;
    using VertexFormat = engine::graphics::VertexFormat;
    using VertexInputRate = engine::graphics::VertexInputRate;

public:
    GLVertexArray(GLVertexArray&& other);
    ~GLVertexArray();

    GLVertexArray() = delete;
    GLVertexArray(const GLVertexArray&) = delete;
    GLVertexArray& operator=(const GLVertexArray&) = delete;
    GLVertexArray& operator=(GLVertexArray&&) = delete;

    GLuint get_handle() {
        return handle;
    }

    GLContext& get_context() {
        return context;
    }

    bool has_index_buffer() {
        return index_format.has_value();
    }

    IndexFormat get_index_format() {
        assert(index_format.has_value());
        return index_format.value();
    }

    // binds a buffer to this vertex array as a vertex buffer 
    // this just tells opengl that the buffer is related to this vertex array on this slot
    // you need to configure a vertex attribute on this vertex array to actually use the information
    //   stored within this buffer
    void gl_bind_vertex_buffer(
        GLuint slot,
        GLBuffer& buffer,
        GLintptr offset,
        GLsizei stride,
        VertexInputRate rate
    );

    // creates a vertex attribute, binds it to the given buffer slot, and enables it
    // a vertex attribute tells opengl how to interpret the underlying buffer
    // 'attrib_slot' will correspond to the underlying shader interface variable location
    // only handles VertexFormats that describe one slot of information like Float32x2, 
    //   will not work with stuff like Float32x4x4
    void gl_bind_vertex_attribute(
        GLuint attrib_slot,
        GLuint buffer_slot,
        VertexFormat format,
        GLuint relative_offset
    );

    // binds a buffer to this vertex array as an index buffer
    void gl_bind_index_buffer(
        GLBuffer& buffer,
        IndexFormat index_format
    );

private:
    GLVertexArray(
        GLGraphicsDevice& device,
        GLContext& context,
        GLuint handle
    ) : device(device), context(context), handle(handle) {}

    // the device that created this vertex array
    GLGraphicsDevice& device;

    // the opengl context this vertex array belongs to
    GLContext& context;

    // OpenGL resource handle
    GLuint handle;

    // this should have a value if there is an index buffer bound to this vertex array
    std::optional<IndexFormat> index_format;
};

}