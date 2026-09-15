#include <engine/graphics/opengl/GLProgram.h>
#include <engine/graphics/opengl/OpenGLGraphicsDevice.h>

namespace engine::graphics::opengl {

GLComputeProgram::GLComputeProgram(GLComputeProgram&& other) :
device(other.device), handle(other.handle) {
    other.handle = 0;
}

GLGraphicsProgram::GLGraphicsProgram(GLGraphicsProgram&& other) :
device(other.device), handle(other.handle), primitive_topology(other.primitive_topology) {
    other.handle = 0;
}

GLComputeProgram::~GLComputeProgram() {
    if(handle != 0) {
        device.gl_delete_compute_pipeline(handle);
    }
}

GLGraphicsProgram::~GLGraphicsProgram() {
    if(handle != 0) {
        device.gl_delete_graphics_pipeline(handle);
    }
}



}

