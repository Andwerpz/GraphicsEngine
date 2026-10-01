#include <engine/graphics/ShaderResource.h>
#include <engine/graphics/ShaderType.h>
#include <cassert>

namespace engine::graphics {

ShaderBlockVariable::ShaderBlockVariable() {
    offset = 0;
    size = 0;
    array_info.stride = 0;
}

ShaderBlockVariable::ShaderBlockVariable(const ShaderBlockVariable& other) {
    name = other.name;
    offset = other.offset;
    size = other.size;
    for(int i = 0; i < other.struct_info.members.size(); i++) {
        struct_info.members.emplace_back(other.struct_info.members[i]);
    }
    array_info.stride = other.array_info.stride;
    for(int i = 0; i < other.array_info.dimensions.size(); i++) {
        array_info.dimensions.emplace_back(other.array_info.dimensions[i]);
    }
}

ShaderBlockVariable& ShaderBlockVariable::operator=(const ShaderBlockVariable& other) {
    if(this == &other) return *this;
    name = other.name;
    offset = other.offset;
    size = other.size;
    struct_info.members.clear();
    for(int i = 0; i < other.struct_info.members.size(); i++) {
        struct_info.members.emplace_back(other.struct_info.members[i]);
    }
    array_info.stride = other.array_info.stride;
    array_info.dimensions.clear();
    for(int i = 0; i < other.array_info.dimensions.size(); i++) {
        array_info.dimensions.emplace_back(other.array_info.dimensions[i]);
    }
    return *this;
}

bool ShaderBlockVariable::operator==(const ShaderBlockVariable& other) const {
    if(this == &other) return true;
    if(name != other.name) return false;
    if(offset != other.offset) return false;
    if(size != other.size) return false;
    if(struct_info.members.size() != other.struct_info.members.size()) return false;
    for(int i = 0; i < struct_info.members.size(); i++) {
        if(!(struct_info.members[i] == other.struct_info.members[i])) return false;
    }
    if(array_info.stride == other.array_info.stride) return false;
    for(int i = 0; i < array_info.dimensions.size(); i++) {
        if(array_info.dimensions[i] != other.array_info.dimensions[i]) return false;
    }
    return true;
}

ShaderResource::UniformBuffer::UniformBuffer() :
    type(ShaderVoidType()),
    blockvar()
{
    // do nothing
}

ShaderResource::UniformBuffer::UniformBuffer(
    ShaderType type,
    ShaderBlockVariable blockvar
) : type(type), 
    blockvar(blockvar)
{
    // do nothing
}

ShaderResource::SampledTexture::SampledTexture(
    ShaderImageType image_type
) : image_type(image_type)
{
    // do nothing
}

ShaderResource::SampledTexture::SampledTexture() :
    image_type(ShaderVoidType(), TextureDimension::D2, false, false)
{
    // do nothing
}


ShaderResource ShaderResource::create_uniform_buffer(
    std::string name,
    ShaderType type,
    ShaderBlockVariable blockvar
) {
    ShaderResource res;
    res.base_type = ShaderResourceType::UniformBuffer;
    res.name = name;
    res.uniform_buffer = UniformBuffer(type, blockvar);
    return res;
}

ShaderResource ShaderResource::create_sampled_texture(
    std::string name,
    ShaderImageType image_type
) {
    ShaderResource res;
    res.base_type = ShaderResourceType::SampledTexture;
    res.name = name;
    res.sampled_texture = SampledTexture(image_type);
    return res;
}

bool ShaderResource::operator==(const ShaderResource& other) const {
    if(this == &other) return true;
    if(base_type != other.base_type) return false;
    if(name != other.name) return false;
    switch(base_type) {
        case ShaderResourceType::UniformBuffer: {
            if(!(uniform_buffer.type == other.uniform_buffer.type)) return false;
            if(!(uniform_buffer.blockvar == other.uniform_buffer.blockvar)) return false;
            break;
        }
        case ShaderResourceType::SampledTexture: {
            if(!(sampled_texture.image_type == other.sampled_texture.image_type)) return false;
            break;
        }
        default:
            assert(false);
    }
    return true;
}

}