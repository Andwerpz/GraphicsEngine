#include <engine/graphics/ProgramInvocation.h>
#include <engine/graphics/Program.h>
#include <engine/graphics/ShaderInterface.h>
#include <engine/graphics/ShaderResource.h>
#include <engine/graphics/GraphicsDevice.h>
#include <engine/graphics/Buffer.h>
#include <engine/utils/error.h>

#include <cassert>
#include <stdexcept>
#include <iostream>

namespace engine::graphics {

ProgramInvocation::ProgramInvocation(
    GraphicsDevice& device,
    Program& program
) : device(device),
    program(program) 
{   
    ShaderInterface& program_interface = program.get_interface();
    for(auto i = program_interface.resources.begin(); i != program_interface.resources.end(); i++) {
        auto location = i->first;
        ShaderResource& resource_interface = i->second;
        switch(resource_interface.base_type) {
            case ShaderResourceType::UniformBuffer: {
                // generate a default uniform buffer
                uint64_t buffer_size = resource_interface.uniform_buffer.blockvar.size;
                auto buffer = device.create_buffer(
                    buffer_size,
                    BufferUsage::Uniform,
                    BufferFrequency::Dynamic
                );
                bind_uniform_buffer(location, *buffer);
                uniform_buffers.emplace_back(std::move(buffer));
                break;
            }
        }
    }
}

ProgramInvocation::~ProgramInvocation() = default;

void ProgramInvocation::bind_uniform_buffer(ShaderResourceLocation location, Buffer& uniform_buffer) {
    // check underlying resource binding
    ShaderInterface& program_interface = program.get_interface();
    if(program_interface.get_resource_type(location) != ShaderResourceType::UniformBuffer) {
        engine::utils::throw_error("bind_uniform_buffer underlying resource descriptor is not UniformBuffer");
    }

    // given buffer needs to be of type uniform
    if(uniform_buffer.get_usage() != BufferUsage::Uniform) {
        engine::utils::throw_error("buffer needs to be a uniform buffer");
    }

    // make sure buffer is large enough
    ShaderResource& resource_interface = program_interface.get_resource(location);
    uint64_t required_size = resource_interface.uniform_buffer.blockvar.size;
    if(uniform_buffer.get_size() < required_size) {
        engine::utils::throw_error("uniform buffer not large enough, required size : " + std::to_string(required_size));
    }

    // bind resource
    bind_resource(location, ResourceBinding::create_uniform_buffer(uniform_buffer));
}

void ProgramInvocation::bind_sampled_texture(ShaderResourceLocation location, Texture& texture, Sampler& sampler) {
    ShaderResource& resource = program.get_interface().get_resource(location);
    
    // check underlying resource binding
    if(resource.base_type != ShaderResourceType::SampledTexture) {
        engine::utils::throw_error("bind_sampled_texture underlying resource descriptor is not SampledTexture");
    }

    // make sure texture dimension is correct
    ShaderImageType& image_type = resource.sampled_texture.image_type;
    if(image_type.dimension != texture.get_dimension()) {
        engine::utils::throw_error("bind_sampled_texture texture dimension mismatch");
    }
    
    // bind resource
    bind_resource(location, ResourceBinding::create_sampled_texture(texture, sampler));
}

Buffer& ProgramInvocation::get_uniform_buffer(ShaderResourceLocation location) {
    ResourceBinding& resource = get_resource(location);
    if(resource.type != ShaderResourceType::UniformBuffer) {
        engine::utils::throw_error("get_uniform_buffer requires underlying resource to be of type UniformBuffer");
    }
    assert(resource.uniform_buffer.buffer != nullptr);
    return *resource.uniform_buffer.buffer;
}

template<typename T>
UniformToken<T> ProgramInvocation::get_uniform(std::string_view _addr) {
    std::string addr(_addr);

    // make sure we've validated
    assert(validated);

    // parsing methods, these strip off the relevant parts and modify the original string
    auto get_struct_member_name = [](std::string& addr) -> std::string {
        // make sure addr isn't empty
        if(addr.size() == 0) {
            engine::utils::throw_error("empty addr when getting struct member name");
        }
        // everything before the first '.', '[', or end of string, is the member name
        std::string retval;
        for(int i = 0; i < addr.size(); i++) {
            if(addr[i] == '.') {
                // strip the period off as well
                retval = addr.substr(0, i);
                addr = addr.substr(i + 1);
                return retval;
            }
            if(addr[i] == '[') {
                // don't strip the opening bracket
                retval = addr.substr(0, i);
                addr = addr.substr(i);
                return retval;
            }
        }
        retval = addr;
        addr = "";
        return retval;
    };
    auto get_array_index = [](std::string& addr) -> uint32_t {
        // make sure addr isn't empty
        if(addr.size() == 0) {
            engine::utils::throw_error("empty addr when getting array index");
        }
        // make sure that the first char is '['
        if(addr[0] != '[') {
            engine::utils::throw_error("first char should be '['");
        }
        // parse until we see a ']'
        bool found_index = false;
        std::string index_str;
        for(int i = 1; i < addr.size(); i++) {
            if(addr[i] == ']') {
                found_index = true;
                index_str = addr.substr(1, i - 1);
                addr = addr.substr(i + 1);
                break;
            }
        }
        if(!found_index) {
            engine::utils::throw_error("could not find closing bracket");
        }
        // try to convert index string into unsigned integer
        try {
            return std::stoull(index_str);
        } catch(std::runtime_error e) {
            engine::utils::throw_error("failed to convert index str : " + std::string(e.what()));
        }
    };

    // figure out which uniform buffer this address refers to
    auto get_buffer_name = [&](std::string& addr) -> std::string {
        try {
            return get_struct_member_name(addr);
        } catch(std::runtime_error e) {
            engine::utils::throw_error("addr does not refer to a buffer");
        }
    };
    auto get_buffer_location = [&](std::string& addr) -> std::pair<uint32_t, uint32_t> {
        // find the resource in the interface that has name equal to the addr buffer name
        std::string buffer_name = get_buffer_name(addr);
        ShaderInterface& interface = program.get_interface();
        for(auto i = interface.resources.begin(); i != interface.resources.end(); i++) {
            if(i->second.name == buffer_name) return i->first;
        }
        engine::utils::throw_error("addr refers to a resource that does not exist : \"" + buffer_name + "\"");
    };

    // make sure location corresponds to a uniform buffer
    auto location = get_buffer_location(addr);
    ResourceBinding& resource = get_resource(location);
    if(resource.type != ShaderResourceType::UniformBuffer) {
        engine::utils::throw_error("addr refers to a resource that is not a uniform buffer");
    }

    // get blockvar
    ShaderResource& interface_resource = program.get_interface().resources.at(location);
    assert(interface_resource.base_type == ShaderResourceType::UniformBuffer);
    ShaderType* type = &interface_resource.uniform_buffer.type;
    ShaderBlockVariable* blockvar = &interface_resource.uniform_buffer.blockvar;

    // figure out byte offset into buffer
    uint64_t offset = 0;
    while(true) {
        // array indexing
        if(blockvar->array_info.dimensions.size() != 0) {
            uint64_t curr_offset = 0;
            std::vector<uint64_t>& dimensions = blockvar->array_info.dimensions;
            for(int i = 0; i < dimensions.size(); i++) {
                assert(std::holds_alternative<ShaderArrayType>(type->type));
                ShaderArrayType& array_type = std::get<ShaderArrayType>(type->type);
                assert(dimensions[i] == array_type.dimension);

                // get array index, make sure it isn't out of bounds
                uint32_t array_index = get_array_index(addr);
                if(array_index >= dimensions[i]) {
                    engine::utils::throw_error("array index " + std::to_string(array_index) + " out of bounds");
                }

                // index into array
                curr_offset = (curr_offset * dimensions[i]) + array_index;
                type = array_type.base_type.get();
            }
            curr_offset *= blockvar->array_info.stride;
            offset += curr_offset;
        }

        // struct members
        if(blockvar->struct_info.members.size() != 0) {
            std::vector<ShaderBlockVariable>& members = blockvar->struct_info.members;
            assert(std::holds_alternative<ShaderStructType>(type->type));
            ShaderStructType& struct_type = std::get<ShaderStructType>(type->type);
            assert(members.size() == struct_type.members.size());
            
            // find member with name
            bool found_member = false;
            std::string member_name = get_struct_member_name(addr);
            for(int i = 0; i < members.size(); i++) {
                ShaderBlockVariable& member = members[i];
                if(member.name == member_name) {
                    assert(struct_type.members[i].name == member_name);
                    found_member = true;
                    type = struct_type.members[i].type.get();
                    offset += member.offset;
                    blockvar = &member;
                    break;
                }
            }
            if(!found_member) {
                engine::utils::throw_error("unable to find member with name : \"" + member_name + "\"");
            }
        }

        // see if we should stop
        // should stop if type is some scalar, vector, or matrix type 
        if(
            std::holds_alternative<ShaderScalarType>(type->type) ||
            std::holds_alternative<ShaderVectorType>(type->type) ||
            std::holds_alternative<ShaderMatrixType>(type->type) 
        ) {
            break;
        }
    }

    // validate that the type is appropriate for the uniform variable being referenced
    bool is_valid = false;
    auto type_error = [&type](std::string exp_type) -> void {
        engine::utils::throw_error("type mismatch, expected " + exp_type + ", got " + to_string(*type));
    };
    if constexpr(std::is_same_v<T, float>) {
        if(!std::holds_alternative<ShaderScalarType>(type->type)) {
            type_error("float");
        }
        if(std::get<ShaderScalarType>(type->type) != ShaderScalarType::Float) {
            type_error("float");
        }
        is_valid = true;
    }
    else if constexpr(std::is_same_v<T, glm::vec3>) {
        if(!std::holds_alternative<ShaderVectorType>(type->type)) {
            type_error("vec3");
        }
        ShaderVectorType& vector_type = std::get<ShaderVectorType>(type->type);
        if(vector_type.base_type != ShaderScalarType::Float || vector_type.dimension != 3) {
            type_error("vec3");
        }
        is_valid = true;
    }
    else if constexpr(std::is_same_v<T, glm::mat4>) {
        if(!std::holds_alternative<ShaderMatrixType>(type->type)) {
            type_error("mat4");
        }
        ShaderMatrixType& matrix_type = std::get<ShaderMatrixType>(type->type);
        if(
            matrix_type.base_type != ShaderScalarType::Float || 
            matrix_type.rows != 4 || matrix_type.cols != 4
        ) {
            type_error("mat4");
        }
        is_valid = true;
    }
    else {
        engine::utils::throw_error("invalid type");
    }

    // create token
    UniformToken<T> token;
    token.offset = offset;
    token.location = location;
    return token;
}

static_assert(sizeof(float) == 4);
template UniformToken<float>
ProgramInvocation::get_uniform<float>(std::string_view);
void ProgramInvocation::set_uniform(UniformToken<float> token, float val) {
    Buffer& buffer = get_uniform_buffer(token.location);
    assert(buffer.get_usage() == BufferUsage::Uniform);
    buffer.upload_data(&val, 4, token.offset);
}

static_assert(sizeof(glm::vec3) == 4 * 3);
template UniformToken<glm::vec3>
ProgramInvocation::get_uniform<glm::vec3>(std::string_view);
void ProgramInvocation::set_uniform(UniformToken<glm::vec3> token, glm::vec3 val) {
    Buffer& buffer = get_uniform_buffer(token.location);
    assert(buffer.get_usage() == BufferUsage::Uniform);
    buffer.upload_data(&val, 4 * 3, token.offset);
}

static_assert(sizeof(glm::mat4) == 4 * 4 * 4);
template UniformToken<glm::mat4>
ProgramInvocation::get_uniform<glm::mat4>(std::string_view);
void ProgramInvocation::set_uniform(UniformToken<glm::mat4> token, glm::mat4 val) {
    Buffer& buffer = get_uniform_buffer(token.location);
    assert(buffer.get_usage() == BufferUsage::Uniform);
    buffer.upload_data(&val, 4 * 4 * 4, token.offset);
}

void ProgramInvocation::validate() {
    // make sure we haven't validated
    assert(!validated);

    validate_impl();
    validated = true;
}

void ProgramInvocation::bind_resource(ShaderResourceLocation location, ResourceBinding resource) {
    bound_resources[location] = resource;
}

ProgramInvocation::ResourceBinding& ProgramInvocation::get_resource(ShaderResourceLocation location) {
    if(!bound_resources.contains(location)) {
        engine::utils::throw_error("no resource at location");
    }
    return bound_resources.at(location);
}

void ProgramInvocation::validate_resource_bindings() {
    // for now, just make sure that the number of available resource binding slots 
    // matches the amount of entries in bound_resources
    ShaderInterface& interface = program.get_interface();
    assert(bound_resources.size() <= interface.resources.size());
    if(bound_resources.size() != interface.resources.size()) {
        engine::utils::throw_error("program invocation has unbound resource");
    }
}


RenderingProgramInvocation::RenderingProgramInvocation(
    GraphicsDevice& device,
    Program& program
) : ProgramInvocation(device, program) {
    // do nothing
}

void RenderingProgramInvocation::bind_vertex_attribute(
    uint32_t location,
    Buffer& buffer,
    uint32_t offset,
    uint32_t stride,
    VertexInputRate rate,
    VertexFormat format
) {
    // make sure we haven't validated
    assert(!validated);

    // make sure that this location hasn't been bound to yet
    uint32_t location_count = device.get_vertex_format_location_count(format);
    for(uint32_t i = 0; i < location_count; i++) {
        if(occupied_locations.contains(location + i)) {
            engine::utils::throw_error("location already bound : " + std::to_string(location + i));
        }
    }

    // make sure the buffer is a vertex buffer
    if(buffer.get_usage() != BufferUsage::Vertex) {
        engine::utils::throw_error("buffer binding to vertex attribute needs to be a vertex buffer");
    }

    // add binding
    attribute_bindings.insert({location, (VertexAttributeBinding) {
        buffer,
        offset,
        stride,
        rate,
        format
    }});
    for(uint32_t i = 0; i < location_count; i++) {
        occupied_locations.insert(location + i);
    }
}

void RenderingProgramInvocation::bind_index_buffer(
    Buffer& buffer,
    IndexFormat format
) {
    // make sure we haven't validated
    assert(!validated);
    
    // make sure we haven't bound an index buffer already
    if(index_buffer.has_value()) {
        engine::utils::throw_error("bind_index_buffer index buffer already bound");
    }

    // make sure the buffer is an index buffer
    if(buffer.get_usage() != BufferUsage::Index) {
        engine::utils::throw_error("bind_index_buffer buffer needs to be an index buffer");
    }

    // add binding
    index_buffer.emplace((IndexBufferBinding) {
        buffer,
        format
    });
}

void RenderingProgramInvocation::bind_render_target(
    RenderTarget& target
) {
    // make sure we haven't validated
    assert(!validated);

    // make sure render target is validated
    if(!target.is_validated()) {
        engine::utils::throw_error("render target must be validated before being bound to program invocation");
    }

    // check bound textures against interface
    // make sure all output interface variables have a corresponding output in the target
    for(const auto& [location, output_var] : program.get_interface().outputs) {
        if(!target.has_output(location)) {
            engine::utils::throw_error("render target does not have an output for variable : " + output_var.name);
        }

        // TODO make sure type of output is compatible?
    }

    // save render target
    if(target.has_depth_buffer()) {
        depth_testing = true;
    }
    render_target.emplace(target);
}

void RenderingProgramInvocation::draw(
    uint32_t nr_primitives, 
    uint32_t nr_instances,
    uint32_t first_primitive,
    uint32_t first_instance
) {
    // make sure we have validated
    assert(validated);
    validate_resource_bindings();

    // call subclass impl
    draw_impl(
        nr_primitives,
        nr_instances,
        first_primitive,
        first_instance
    );
}

void RenderingProgramInvocation::do_depth_testing(bool val) {
    depth_testing = val;
}

void RenderingProgramInvocation::do_clear_color(bool val) {
    if(val) clear_flags |= ClearFlags::Color;
    else clear_flags |= ~ClearFlags::Color;
}

void RenderingProgramInvocation::do_clear_depth(bool val) {
    if(val) clear_flags |= ClearFlags::Depth;
    else clear_flags |= ~ClearFlags::Depth;
}

}