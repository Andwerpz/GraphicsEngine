#pragma once
#include <cstddef>
#include <string_view>

namespace engine::asset {

// an EmbeddedResource is one that is built into the engine 
// these are represented as static byte arrays in C++ source code 

struct EmbeddedResource {
    const void* data = nullptr;
    size_t size = 0;
    explicit operator bool() const { return data != nullptr; }
    static EmbeddedResource load(std::string_view path);
};

}