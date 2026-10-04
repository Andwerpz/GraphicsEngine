#pragma once

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

    // vertex array gets built when this invocation gets validated
    // when drawing, this vertex array gets bound to supply vertex information to the program
    std::optional<GLVertexArray> gl_vertex_array;

    // framebuffer retrieved from the render target when this invocation gets validated
    std::optional<std::reference_wrapper<GLFramebuffer>> gl_framebuffer;

    void validate_impl() override;

	void draw_impl(
        uint32_t nr_elements, 
        uint32_t nr_instances
    ) override;
};

class GLComputeProgramInvocation : public engine::graphics::ComputeProgramInvocation {
    // TODO
};

}