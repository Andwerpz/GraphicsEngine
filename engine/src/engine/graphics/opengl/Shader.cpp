#include <engine/graphics/opengl/Shader.h>
#include <engine/graphics/opengl/GraphicsDevice.h>

namespace engine::graphics::opengl {

Shader::~Shader() {
    device->delete_shader(handle);
}

}

