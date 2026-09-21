#pragma once
#include <cstdint>
#include <glad/glad.h>
#include <vector>

#include "../Shader.h"

namespace engine::graphics::opengl {

// a GLShader represents some compiled code on the GPU 

class GLGraphicsDevice;

class GLShader : public engine::graphics::Shader {
    friend class GLGraphicsDevice;
    using ShaderStage = engine::graphics::ShaderStage;

public:
    GLShader(GLShader&&);
    ~GLShader();

    GLuint get_handle() {
        return handle;
    }

private:
    GLShader(
        ShaderStage stage,
        std::string source_code,
        std::vector<uint32_t> spirv_code,
        GLGraphicsDevice& device, 
        GLuint handle
    );

    GLShader() = delete;
    GLShader(const GLShader&) = delete;
    GLShader& operator=(const GLShader&) = delete;
    GLShader& operator=(GLShader&&) = delete;

    // the device that created this shader
    GLGraphicsDevice& device;

    // OpenGL resource handle
    GLuint handle;
};

}