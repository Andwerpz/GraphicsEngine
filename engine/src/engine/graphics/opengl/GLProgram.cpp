#include <engine/graphics/opengl/GLProgram.h>
#include <engine/graphics/opengl/GLGraphicsDevice.h>
#include <engine/graphics/opengl/GLProgramInvocation.h>
#include <engine/graphics/Program.h>

namespace engine::graphics::opengl {

GLRenderingProgram::GLRenderingProgram(GLRenderingProgram&& other) :
engine::graphics::RenderingProgram(std::move(other)), device(other.device), handle(other.handle) {
    other.handle = 0;
}

GLRenderingProgram::~GLRenderingProgram() {
    if(handle != 0) {
        device.gl_delete_rendering_program(*this);
    }
}

GLRenderingProgram::GLRenderingProgram(
    Shader& vertex,
    Shader& fragment,
    PrimitiveTopology primitive_topology,
    GLGraphicsDevice& device, 
    GLuint handle
) : engine::graphics::RenderingProgram(vertex, fragment, primitive_topology), device(device), handle(handle) {
    // do nothing
}

std::unique_ptr<engine::graphics::RenderingProgramInvocation> GLRenderingProgram::create_invocation() {
    return std::unique_ptr<engine::graphics::RenderingProgramInvocation>(
        new GLRenderingProgramInvocation(device, *this)
    );
}


GLComputeProgram::GLComputeProgram(GLComputeProgram&& other) :
engine::graphics::ComputeProgram(std::move(other)), device(other.device), handle(other.handle) {
    other.handle = 0;
}

GLComputeProgram::~GLComputeProgram() {
    if(handle != 0) {
        device.gl_delete_compute_program(*this);
    }
}

GLComputeProgram::GLComputeProgram(
    Shader& compute,
    GLGraphicsDevice& device, 
    GLuint handle
) : ComputeProgram(compute), device(device), handle(handle) {
    // do nothing
}

std::unique_ptr<engine::graphics::ComputeProgramInvocation> GLComputeProgram::create_invocation() {
    // TODO
}

}

