#include <engine/graphics/opengl/GLVertexArray.h>
#include <engine/graphics/opengl/GLGraphicsDevice.h>
#include <glad/glad.h>

namespace engine::graphics::opengl {

GLVertexArray::GLVertexArray(
    GLGraphicsDevice& device,
    GLuint handle
) : device(device),
    handle(handle)
{
    // do nothing
}

GLVertexArray::GLVertexArray(GLVertexArray&& other) :
    device(other.device),
    handle(other.handle) 
{
    other.handle = 0;
}

GLVertexArray::~GLVertexArray() {
    if(handle != 0) {
        device.gl_delete_vertex_array(handle);
    }
}

void GLVertexArray::gl_bind_vertex_buffer(
    GLuint slot,
    GLBuffer& buffer,
    GLintptr offset,
    GLsizei stride
) {
    glVertexArrayVertexBuffer(
        handle,
        slot,
        buffer.get_handle(),
        offset,
        stride
    );
}

void GLVertexArray::gl_bind_index_buffer(
    GLBuffer& buffer,
    IndexFormat format
) { 
    assert(!index_format.has_value());
    glVertexArrayElementBuffer(
        handle, 
        buffer.get_handle()
    );
    index_format = format;
}

}