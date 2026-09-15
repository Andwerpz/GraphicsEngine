#include <engine/graphics/opengl/GLShader.h>
#include <engine/graphics/opengl/OpenGLGraphicsDevice.h>

namespace engine::graphics::opengl {

GLShader::GLShader(GLShader&& other) :
device(other.device), handle(other.handle), type(other.type) {
    other.handle = 0;
}

GLShader::~GLShader() {
    if(handle != 0) {
        device.gl_delete_shader(handle);
    }
}

}

