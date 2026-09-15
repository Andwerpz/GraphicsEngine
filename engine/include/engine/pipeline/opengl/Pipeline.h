#pragma once
#include "Shader.h"
#include "../Pipeline.h"

namespace engine::pipeline::opengl {

class Pipeline : public engine::pipeline::Pipeline {
public:
    Pipeline(engine::graphics::opengl::OpenGLGraphicsDevice& device);

private:
    engine::graphics::opengl::OpenGLGraphicsDevice& device;
};

}