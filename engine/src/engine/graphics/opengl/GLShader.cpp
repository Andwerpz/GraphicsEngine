#include <engine/graphics/opengl/GLShader.h>
#include <engine/graphics/opengl/GLGraphicsDevice.h>

namespace engine::graphics::opengl {

GLShader::GLShader(GLShader&& other) : 
    engine::graphics::Shader(std::move(other)),
    device(other.device),
    handle(other.handle) {
    other.handle = 0;
}

GLShader::~GLShader() {
    if(handle != 0) {
        device.gl_delete_shader(*this);
    }
}

GLShader::GLShader(
    ShaderStage stage,
    std::string source_code,
    std::vector<uint32_t> spirv_code,
    GLGraphicsDevice& device, 
    GLuint handle
) : engine::graphics::Shader(stage, source_code, spirv_code),
    device(device),
    handle(handle) {
    // do nothing
}

}

