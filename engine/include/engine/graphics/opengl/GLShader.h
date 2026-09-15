#pragma once
#include <cstdint>
#include <glad/glad.h>

namespace engine::graphics::opengl {

// a GLShader represents some compiled code on the GPU 
// the type of a shader determines what sort of pipelines it can participate in, and in what stages

class OpenGLGraphicsDevice;

enum class GLShaderType {
    Vertex,
    Fragment,
    Compute,
};

class GLShader {
public:
    GLShader(GLShader&&);
    ~GLShader();

    GLShaderType get_type() const {
        return type;
    }

private:
    friend class OpenGLGraphicsDevice;

    GLShader(OpenGLGraphicsDevice& device, GLuint handle, GLShaderType type) :
        device(device), handle(handle), type(type) {}

    GLShader() = delete;
    GLShader(const GLShader&) = delete;
    GLShader& operator=(const GLShader&) = delete;
    GLShader& operator=(GLShader&&) = delete;

    // the device that created this shader
    OpenGLGraphicsDevice& device;

    // OpenGL resource handle
    GLuint handle;

    // what sort of shader is this
    // used by pipelines to make sure that you're giving them valid shaders 
    GLShaderType type;
};

}