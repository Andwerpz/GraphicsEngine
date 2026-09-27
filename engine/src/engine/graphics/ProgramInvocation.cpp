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
    // do nothing
}

ProgramInvocation::~ProgramInvocation() = default;

void ProgramInvocation::bind_resource(uint32_t set, uint32_t binding, Resource& resource) {
    bind_resource({set, binding}, resource);
}

void ProgramInvocation::bind_resource(std::pair<uint32_t, uint32_t> location, Resource& resource) {
    // make sure we haven't validated
    assert(!validated);

    // can't bind multiple resources to the same location
    if(resources.contains(location)) {
        engine::utils::throw_error("resource already bound to location");
    }

    resources.insert({location, resource});
}

Resource& ProgramInvocation::get_resource(uint32_t set, uint32_t binding) {
    return get_resource({set, binding});
}

Resource& ProgramInvocation::get_resource(std::pair<uint32_t, uint32_t> location) {
    if(!resources.contains(location)) {
        engine::utils::throw_error("no resource at location");
    }
    return resources.at(location);
}

void ProgramInvocation::validate() {
    // make sure we haven't validated
    assert(!validated);

    // validate resources
    ShaderInterface& program_interface = program.get_interface();
    for(auto i = program_interface.resources.begin(); i != program_interface.resources.end(); i++) {
        auto location = i->first;
        ShaderResource& resource_interface = i->second;
        switch(resource_interface.base_type) {
            case ShaderResourceType::UniformBuffer: {
                // see if we need to generate a buffer
                if(!resources.contains(location)) {
                    uint64_t buffer_size = resource_interface.uniform_buffer.blockvar.size;
                    std::cout << "Creating uniform buffer size : " << buffer_size << std::endl;
                    auto buffer = device.create_buffer(
                        buffer_size,
                        BufferUsage::Uniform,
                        BufferFrequency::Dynamic
                    );
                    bind_resource(location, *buffer);
                    uniform_buffers.emplace_back(std::move(buffer));
                }

                // validate resource
                // - make sure resource is a buffer
                Resource& resource = get_resource(location);
                if(dynamic_cast<Buffer*>(&resource) == nullptr) {
                    engine::utils::throw_error("uniform buffer resource needs to be a buffer");
                }
                Buffer& buffer = dynamic_cast<Buffer&>(resource);

                // - make sure it's a uniform buffer
                if(buffer.get_usage() != BufferUsage::Uniform) {
                    engine::utils::throw_error("buffer bound to uniform resource should be a uniform buffer");
                }

                // - make sure buffer is large enough
                uint64_t buffer_size = resource_interface.uniform_buffer.blockvar.size;
                if(buffer.get_size() < buffer_size) {
                    engine::utils::throw_error("uniform buffer not large enough, required size : " + std::to_string(buffer_size));
                }
                break;
            }
            default:
                assert(false);
        }
    }

    validate_impl();
    validated = true;
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
        std::cout << "Interface resource count : " << interface.resources.size() << std::endl;
        for(auto i = interface.resources.begin(); i != interface.resources.end(); i++) {


            std::cout << "RESOURCE NAME : " << i->second.name << std::endl;
            if(i->second.name == buffer_name) return i->first;
        }
        engine::utils::throw_error("addr refers to a resource that does not exist : \"" + buffer_name + "\"");
    };

    auto location = get_buffer_location(addr);
    assert(dynamic_cast<Buffer*>(&get_resource(location)) != nullptr);
    Buffer& buffer = dynamic_cast<Buffer&>(get_resource(location));
    assert(program.get_interface().resources.contains(location));
    ShaderResource& interface_resource = program.get_interface().resources.at(location);
    assert(interface_resource.base_type == ShaderResourceType::UniformBuffer);
    ShaderType* type = &interface_resource.uniform_buffer.type;
    ShaderBlockVariable* blockvar = &interface_resource.uniform_buffer.blockvar;

    // figure out byte offset into buffer
    uint64_t offset = 0;
    while(true) {
        std::cout << "OFFSET : " << offset << " " << addr << std::endl;

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
    else {
        engine::utils::throw_error("invalid type");
    }

    // create token
    UniformToken<T> token;
    token.offset = offset;
    token.location = location;
    return token;
}

template UniformToken<float>
ProgramInvocation::get_uniform<float>(std::string_view);
void ProgramInvocation::set_uniform(UniformToken<float> token, float val) {
    Resource& resource = get_resource(token.location);
    assert(dynamic_cast<Buffer*>(&resource));
    Buffer& buffer = dynamic_cast<Buffer&>(resource);
    buffer.upload_data(&val, 4, token.offset);
}

static_assert(sizeof(glm::vec3) == 4 * 3);
template UniformToken<glm::vec3>
ProgramInvocation::get_uniform<glm::vec3>(std::string_view);
void ProgramInvocation::set_uniform(UniformToken<glm::vec3> token, glm::vec3 val) {
    Resource& resource = get_resource(token.location);
    assert(dynamic_cast<Buffer*>(&resource));
    Buffer& buffer = dynamic_cast<Buffer&>(resource);
    buffer.upload_data(&val, 4 * 3, token.offset);
}


RenderingProgramInvocation::RenderingProgramInvocation(
    GraphicsDevice& device,
    Program& program
) : ProgramInvocation(device, program) {
    // do nothing
}

void RenderingProgramInvocation::bind_vertex_attribute(
    Buffer& buffer,
    uint32_t location,
    uint32_t offset,
    uint32_t stride,
    VertexInputRate rate,
    VertexFormat format
) {
    // make sure we haven't validated
    assert(!validated);

    // make sure that this location hasn't been bound to yet
    for(VertexAttributeBinding& binding : attribute_bindings) {
        if(binding.location == location) {
            engine::utils::throw_error("bind_vertex_attribute location already bound");
        }
    }

    // make sure the buffer is a vertex buffer
    if(buffer.get_usage() != BufferUsage::Vertex) {
        engine::utils::throw_error("buffer binding to vertex attribute needs to be a vertex buffer");
    }

    // add binding
    attribute_bindings.emplace_back((VertexAttributeBinding) {
        buffer,
        location,
        offset,
        stride,
        rate,
        format
    });
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

void RenderingProgramInvocation::bind_output_texture(
    uint32_t location,
    Texture& texture
) {
    // make sure we haven't validated
    assert(!validated);

    // make sure we haven't bound the window
    assert(!bound_window);

    // make sure we haven't bound a texture to this location
    for(OutputBinding& binding : output_bindings) {
        if(binding.location == location) {
            engine::utils::throw_error("bind_output_texture location already bound");
        }
    }

    // add binding
    output_bindings.emplace_back((OutputBinding) {
        location, texture
    });
}

void RenderingProgramInvocation::bind_window() {
    // make sure we haven't validated
    assert(!validated);

    // make sure that we haven't bound any textures
    assert(output_bindings.size() == 0);
    
    bound_window = true;
}

void RenderingProgramInvocation::draw(
    uint32_t nr_elements, 
    uint32_t nr_instances
) {
    // make sure we have validated
    assert(validated);

    // call subclass impl
    draw_impl(
        nr_elements,
        nr_instances
    );
}

}