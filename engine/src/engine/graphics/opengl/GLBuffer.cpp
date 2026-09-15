#include <engine/graphics/opengl/GLBuffer.h>
#include <engine/graphics/opengl/OpenGLGraphicsDevice.h>

namespace engine::graphics::opengl {

GLBuffer::GLBuffer(GLBuffer&& other) : 
device(other.device), handle(other.handle), size(other.size), memory(other.memory) {
    other.handle = 0;
}

GLBuffer::~GLBuffer() {
    if(handle != 0) {
        device.gl_delete_buffer(handle);
    }
}

}

