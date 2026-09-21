#pragma once
#include <glad/glad.h>
#include <cassert>
#include <optional>

#include "../ProgramInvocation.h"

namespace engine::graphics::opengl {

class GLGraphicsDevice;
class GLBuffer;

class GLVertexArray {
    friend class GLGraphicsDevice;
    using IndexFormat = engine::graphics::IndexFormat;

private:
    GLVertexArray(
        GLGraphicsDevice& device,
        GLuint handle
    );

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

    bool has_index_buffer() {
        return index_format.has_value();
    }

    IndexFormat get_index_format() {
        assert(index_format.has_value());
        return index_format.value();
    }

    // binds a buffer to this vertex array as a vertex buffer 
    void gl_bind_vertex_buffer(
        GLuint slot,
        GLBuffer& buffer,
        GLintptr offset,
        GLsizei stride
    );

    // binds a buffer to this vertex array as an index buffer
    void gl_bind_index_buffer(
        GLBuffer& buffer,
        IndexFormat index_format
    );

private:
    // the device that created this texture
    GLGraphicsDevice& device;

    // OpenGL resource handle
    GLuint handle;

    // this should have a value if there is an index buffer bound to this vertex array
    std::optional<IndexFormat> index_format;
};

}