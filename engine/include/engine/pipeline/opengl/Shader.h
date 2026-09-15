#pragma once 
#include "engine/pipeline/Shader.h"
#include "engine/graphics/opengl/GLShader.h"

namespace engine::graphics::opengl {
    class OpenGLGraphicsDevice;
}

namespace engine::pipeline::opengl {

class Shader : public engine::pipeline::Shader {
private:
    friend class engine::graphics::opengl::OpenGLGraphicsDevice;

    Shader(
        std::string source_code,
        std::vector<uint32_t> spirv_code,
        engine::graphics::opengl::OpenGLGraphicsDevice& device, 
        engine::graphics::opengl::GLShader&& shader
    );

    // device that owns underlying GPU shader
    engine::graphics::opengl::OpenGLGraphicsDevice& device;

    // underlying GPU shader resource
    engine::graphics::opengl::GLShader shader;
};

}
