#include <engine/graphics/opengl/RenderTarget.h>
#include <engine/graphics/opengl/GraphicsDevice.h>

namespace engine::graphics::opengl {

RenderTarget::~RenderTarget() {
    device->delete_render_target(handle);
}

}

