#pragma once
#include <string>
#include <optional>

#include "ShaderType.h"

namespace engine::graphics {

enum class ShaderResourceType {
    UniformBuffer,
    SampledTexture,
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

    // requires a buffer of type Uniform
    struct UniformBuffer {
        ShaderType type;
        ShaderBlockVariable blockvar;
        UniformBuffer();
        UniformBuffer(ShaderType type, ShaderBlockVariable blockvar);
    } uniform_buffer;

    // requires a texture and a sampler
    struct SampledTexture {
        ShaderImageType image_type;
        SampledTexture();
        SampledTexture(ShaderImageType image_type);
    } sampled_texture;

    static ShaderResource create_uniform_buffer(
        std::string name, 
        ShaderType uniform_buffer_type,
        ShaderBlockVariable blockvar
    );
    static ShaderResource create_sampled_texture(
        std::string name,
        ShaderImageType image_type
    );

    bool operator==(const ShaderResource& other) const;
};
    
}