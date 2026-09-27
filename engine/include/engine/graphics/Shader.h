#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <memory>

#include "ShaderInterface.h"

namespace engine::graphics {

// a Shader is some code that is meant to run on the GPU 
// shader code should get compiled to SPIR-V so that it can get introspected 

enum class ShaderStage {
    Vertex,
    Fragment,
    Compute,
};

class Shader {
public: 
    Shader(Shader&& other);
    virtual ~Shader() = 0;

protected:
    Shader(
        ShaderStage stage, 
        std::string source_code, 
        std::vector<uint32_t> spirv_code
    );

    // what sort of shader is this?
    ShaderStage stage;

    // original source code
    std::string source_code;

    // compiled SPIR-V code
    std::vector<uint32_t> spirv_code;

    // interface built by introspection
    ShaderInterface interface;

    // called as part of the constructor
    // constructs interface metadata for this shader 
    void introspect();

public:
    ShaderStage get_stage() {
        return stage;
    }

    std::string& get_source_code() {
        return source_code;
    }

    ShaderInterface& get_interface() {
        return interface;
    }
};

}
