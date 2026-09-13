#pragma once
#include <string>

#include <filesystem>
namespace fs = std::filesystem;

namespace engine::pipeline {

// a shader is a program that is meant to run on the GPU 
// this class is pretty much a container for the source code + source introspection
// it shouldn't handle actually interfacing with the underlying graphics API

// for now, assume all source is written in GLSL

class Shader {
public: 
    Shader(fs::path source_path);

    std::string& get_source_code() {
        return source_code;
    }

private:
    std::string source_code;

};

}
