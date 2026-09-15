#pragma once
#include <string>
#include <vector>
#include <cstdint>

#include "engine/graphics/GraphicsDevice.h"

namespace engine::pipeline {

// a Shader is a program that is meant to run on the GPU 
// this class is pretty much a container for the source code + source introspection
// it doesn't manage the underlying 'GPU Shader' which is some compiled code on the GPU 

// a ShaderInterface describes the resources the Shader requires, and what slots they are required on 

// for now, assume all source is written in GLSL
// in the future, want to be able to configure what language it is
//   (GLSL, HLSL, Slang, ...)

class ShaderInterface {
    // TODO
};

enum class ShaderType {
    Vertex,
    Fragment,
    Compute,
};

class Shader {
public: 
    Shader(std::string source_code, std::vector<uint32_t> spirv_code) :
    source_code(source_code), spirv_code(spirv_code) {
        introspect();
    }

    std::string& get_source_code() {
        return source_code;
    }

private:
    ShaderType type;
    std::string source_code;
    std::vector<uint32_t> spirv_code;

    // called as part of the constructor
    // constructs interface metadata for this shader 
    void introspect();
};

}
