#pragma once
#include <cstdint>
#include <glad/glad.h>

#include "../Program.h"
#include "../Shader.h"
#include "../ProgramInvocation.h"

namespace engine::graphics::opengl {

// a GLComputeProgram represents some code that is explicitly meant to be ran with some 
//   general purpose computation kernel

// a GLRenderingProgram is more specialized for geometric primitive rendering
// the program specifies what sort of geometric primitives it expects to be rendering

class GLGraphicsDevice;


class GLRenderingProgram : public engine::graphics::RenderingProgram {
    friend class GLGraphicsDevice;
    using PrimitiveTopology = engine::graphics::PrimitiveTopology;
    using Shader = engine::graphics::Shader;

public:
    GLRenderingProgram(GLRenderingProgram&&);
    ~GLRenderingProgram() override;

    GLuint get_handle() {
        return handle;
    }

    std::unique_ptr<engine::graphics::RenderingProgramInvocation> create_invocation() override;

private:
    GLRenderingProgram(
        Shader& vertex,
        Shader& fragment,
        PrimitiveTopology primitive_topology,
        GLGraphicsDevice& device, 
        GLuint handle
    );

    GLRenderingProgram() = delete;
    GLRenderingProgram(const GLRenderingProgram&) = delete;
    GLRenderingProgram& operator=(const GLRenderingProgram&) = delete;
    GLRenderingProgram& operator=(GLRenderingProgram&&) = delete;

    // the device that created this program
    GLGraphicsDevice& device;

    // OpenGL resource handle
    GLuint handle;
};


class GLComputeProgram : public engine::graphics::ComputeProgram {
    friend class GLGraphicsDevice;

public:
    GLComputeProgram(GLComputeProgram&&);
    ~GLComputeProgram() override;

    GLuint get_handle() {
        return handle;
    }

    std::unique_ptr<engine::graphics::ComputeProgramInvocation> create_invocation() override;

private:
    GLComputeProgram(
        Shader& compute,
        GLGraphicsDevice& device, 
        GLuint handle
    );

    GLComputeProgram() = delete;
    GLComputeProgram(const GLComputeProgram&) = delete;
    GLComputeProgram& operator=(const GLComputeProgram&) = delete;
    GLComputeProgram& operator=(GLComputeProgram&&) = delete;

    // the device that created this program
    GLGraphicsDevice& device;

    // OpenGL resource handle
    GLuint handle;
};

}