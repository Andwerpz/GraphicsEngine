#pragma once

#include "../ProgramInvocation.h"

#include "GLVertexArray.h"
#include "GLFramebuffer.h"

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
	GLGraphicsDevice& device;

	// underlying program 
	GLRenderingProgram& program;

    // vertex array gets built when this invocation gets validated
    // when drawing, this vertex array gets bound to supply vertex information to the program
    std::optional<GLVertexArray> vertex_array;

    // framebuffer gets built when this invocation gets validated
    // all output textures should get bound to this framebuffer
    // when drawing, this framebuffer gets bound and rendered to 
    std::optional<GLFramebuffer> framebuffer;

public:
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