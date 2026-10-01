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
    // bind buffer to vertex array
    glVertexArrayVertexBuffer(
        handle,
        slot,
        buffer.get_handle(),
        offset,
        stride
    );
}

void GLVertexArray::gl_bind_vertex_attribute(
    GLuint attrib_slot,
    GLuint buffer_slot,
    VertexFormat format,
    GLuint relative_offset
) {
    GLuint nr_components;
    GLenum type;
    switch(format) {
        case VertexFormat::Float32x1:
            nr_components = 1;
            type = GL_FLOAT;
            break;
        case VertexFormat::Float32x2:
            nr_components = 2;
            type = GL_FLOAT;
            break;
        case VertexFormat::Float32x3:
            nr_components = 3;
            type = GL_FLOAT;
            break;
        case VertexFormat::Float32x4:
            nr_components = 4;
            type = GL_FLOAT;
            break;
        default:
            assert(false);
    }

    // create new vertex attribute format
    glVertexArrayAttribFormat(
        handle,
        attrib_slot,        // attribute location
        nr_components,      // number of components
        type,               // type
        GL_FALSE,           // normalized
        relative_offset     // byte offset into buffer of the first element
    );

    // link buffer to attribute format
    glVertexArrayAttribBinding(
        handle,
        attrib_slot,        // attrib index
        buffer_slot         // buffer index
    );

    // enable attribute
    glEnableVertexArrayAttrib(
        handle,
        attrib_slot
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