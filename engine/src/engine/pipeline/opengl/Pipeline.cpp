#include <engine/pipeline/opengl/Pipeline.h>
#include <engine/graphics/opengl/OpenGLGraphicsDevice.h>

namespace engine::pipeline::opengl {

Pipeline::Pipeline(engine::graphics::opengl::OpenGLGraphicsDevice& device) : device(device) {
    // do nothing
}

}