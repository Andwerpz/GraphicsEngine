#pragma once
#include "Shader.h"
#include "Buffer.h"
#include "Texture.h"

namespace engine::graphics {
    class GraphicsDevice;
}

namespace engine::pipeline {

// a pipeline is a semantic description of some program that is meant to run on the GPU 
// piplines should be specialized for each underlying graphics API wrapper, and should 
//   interact with them directly in order to actually utilize the GPU 

class Pipeline {
public:
    Pipeline() = default;

    // // -- INITIALIZATION --
    // // adds the next shader in execution order
    // virtual void add_shader(Shader& shader) = 0;

    // // binding resources to shader inputs/outputs
    // virtual void bind_buffer(std::string slot, Shader& shader, Buffer& buf) = 0;
    // virtual void bind_texture(std::string slot, Shader& shader, Texture& tex) = 0;

    // // locks the pipeline into place, no further modifications to the pipeline can be made
    // virtual void construct() = 0;

    // // -- EXECUTION --
    // // must be done after construct()
    // virtual void execute() = 0;
};

}