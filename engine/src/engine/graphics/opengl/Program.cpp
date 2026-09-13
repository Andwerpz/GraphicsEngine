#include <engine/graphics/opengl/Program.h>
#include <engine/graphics/opengl/GraphicsDevice.h>

namespace engine::graphics::opengl {

GraphicsProgram::~GraphicsProgram() {
    device->delete_graphics_pipeline(handle);
}

ComputeProgram::~ComputeProgram() {
    device->delete_compute_pipeline(handle);
}

}

