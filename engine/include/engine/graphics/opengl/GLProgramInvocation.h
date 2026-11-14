#pragma once
#include <optional>
#include <memory>

#include "../ProgramInvocation.h"

#include "GLVertexArray.h"
#include "GLFramebuffer.h"
#include "GLRenderbuffer.h"

namespace engine::graphics {
class Buffer;
class Texture;
}

namespace engine::graphics::opengl {

class GLRenderingProgram;
class GLComputeProgram;
class GLGraphicsDevice;

class GLRenderingProgramInvocation : public engine::graphics::RenderingProgramInvocation {
    friend class GLGraphicsDevice;
    friend class GLRenderingProgram;

private:
    GLRenderingProgramInvocation(
        GLGraphicsDevice& device,
        GLRenderingProgram& program
    );

	// underlying graphics API 
	GLGraphicsDevice& gl_device;

	// underlying program 
	GLRenderingProgram& gl_program;

    // vertex array owned by this invocation
    // the vertex array should be in the same OpenGL context as the supplied render target
    // this is created lazily when drawing as the user can change the bound render target arbitrarily
    std::unique_ptr<GLVertexArray> gl_vertex_array = nullptr;

	void draw_impl(
        uint32_t nr_primitives, 
        uint32_t nr_instances,
        uint32_t first_primitive,
        uint32_t first_instance
    ) override;
};

class GLComputeProgramInvocation : public engine::graphics::ComputeProgramInvocation {
    // TODO
};

}