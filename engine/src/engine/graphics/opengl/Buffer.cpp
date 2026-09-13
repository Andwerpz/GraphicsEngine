#include <engine/graphics/opengl/Buffer.h>
#include <engine/graphics/opengl/GraphicsDevice.h>

namespace engine::graphics::opengl {

Buffer::~Buffer() {
    device->delete_buffer(handle);
}

}

