#include <engine/graphics/opengl/OpenGLGraphicsDevice.h>
#include <engine/graphics/opengl/GLProgram.h>
#include <engine/pipeline/opengl/Shader.h>

namespace engine::pipeline::opengl {

Shader::Shader(
    std::string source_code,
    std::vector<uint32_t> spirv_code,
    engine::graphics::opengl::OpenGLGraphicsDevice& device, 
    engine::graphics::opengl::GLShader&& shader
) : engine::pipeline::Shader(source_code, spirv_code), device(device), shader(std::move(shader)) {}

}