#pragma once
#include <string>
#include <optional>

#include "ShaderType.h"

namespace engine::graphics {

enum class ShaderResourceType {
    UniformBuffer,
    StorageBuffer,
    Sampler,
};

enum class ShaderSamplerDimension {
    D1,
    D2,
    D3,
    Cube,
};

// maps types to byte offsets within a buffer
struct ShaderBlockVariable {
    std::string name;

    // in bytes
    uint64_t offset;    // global offset
    uint64_t size;

    struct StructInfo {
        std::vector<ShaderBlockVariable> members;
    } struct_info;

    struct ArrayInfo {
        uint64_t stride;
        std::vector<uint64_t> dimensions;
    } array_info;

    ShaderBlockVariable();
    ShaderBlockVariable(const ShaderBlockVariable& other);
    ShaderBlockVariable& operator=(const ShaderBlockVariable& other);
    bool operator==(const ShaderBlockVariable& other) const;
};

struct ShaderResource {
    ShaderResourceType base_type;
    std::string name;

    struct UniformBuffer {
        ShaderType type;
        ShaderBlockVariable blockvar;
        UniformBuffer();
        UniformBuffer(ShaderType type, ShaderBlockVariable blockvar);
    } uniform_buffer;

    struct StorageBuffer {
        ShaderType type;
        ShaderBlockVariable blockvar;
        StorageBuffer();
        StorageBuffer(ShaderType type, ShaderBlockVariable blockvar);
    } storage_buffer;

    struct Sampler {
        ShaderSamplerDimension dim;
        Sampler();
    } sampler;

    static ShaderResource create_uniform_buffer(
        std::string name, 
        ShaderType uniform_buffer_type,
        ShaderBlockVariable blockvar
    );
    // static ShaderResource create_sampler(std::string name);

    bool operator==(const ShaderResource& other) const;
};
    
}