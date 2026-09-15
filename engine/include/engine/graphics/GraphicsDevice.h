#pragma once
#include <filesystem>
namespace fs = std::filesystem;

namespace engine::pipeline {
    class Pipeline;
    class Program;
    class Shader;
}

namespace engine::graphics {

class GraphicsDevice {
public:
    // -- PIPELINE --

    // TODO rework this API
    // i don't think GraphicsDevice should be the one responsible for loading and compiling the source code
    // maybe should just pass in raw SPIR-V 
    virtual std::unique_ptr<engine::pipeline::Pipeline> create_pipeline() = 0;
    virtual std::unique_ptr<engine::pipeline::Program> create_program(std::vector<engine::pipeline::Shader*> shaders) = 0;
    virtual std::unique_ptr<engine::pipeline::Shader> create_shader(fs::path source_path) = 0;
};

}

