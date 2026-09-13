#include <engine/pipeline/Shader.h>
#include <cassert>
#include <shaderc/shaderc.h>

namespace engine::pipeline {

Shader::Shader(fs::path source_path) {
    // for now, assume all source code is GLSL
    assert(source_path.extension().string() == "glsl");

    // load source code
    

    // TODO compile to SPIR-V
    
    // TODO do introspection
}

}