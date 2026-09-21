#include <engine/graphics/Shader.h>

#include <spirv_reflect.h>

#include <cassert>
#include <iostream>
#include <fstream>
#include <set>

namespace engine::graphics {

const char* get_stage_string(SpvReflectShaderStageFlagBits spv_stage) {
    switch(spv_stage) {
        case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:                   return "VERTEX";
        case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT:     return "TESSELLATION_CONTROL";
        case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:  return "TESSELLATION_EVALUATION";
        case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:                 return "GEOMETRY";
        case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:                 return "FRAGMENT";
        case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:                  return "COMPUTE";
        default:                                                    return "UNKNOWN";
    }
}

const char* get_result_string(SpvReflectResult spv_result) {
    switch(spv_result) {
        case SPV_REFLECT_RESULT_SUCCESS:                                        return "SUCCESS";
        case SPV_REFLECT_RESULT_NOT_READY:                                      return "NOT_READY";
        case SPV_REFLECT_RESULT_ERROR_PARSE_FAILED:                             return "PARSE_FAILED";
        case SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED:                             return "ALLOC_FAILED";
        case SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED:                           return "RANGE_EXCEEDED";
        case SPV_REFLECT_RESULT_ERROR_NULL_POINTER:                             return "NULL_POINTER";
        case SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR:                           return "INTERNAL_ERROR";
        case SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH:                           return "COUNT_MISMATCH";
        case SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND:                        return "ELEMENT_NOT_FOUND";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE:                  return "SPIRV_INVALID_CODE_SIZE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER:               return "SPIRV_INVALID_MAGIC_NUMBER";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF:                     return "SPIRV_UNEXPECTED_EOF";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE:               return "SPIRV_INVALID_ID_REFERENCE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW:                return "SPIRV_SET_NUMBER_OVERFLOW";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS:              return "SPIRV_INVALID_STORAGE_CLASS";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION:                          return "SPIRV_RECURSION";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION:                return "SPIRV_INVALID_INSTRUCTION";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA:              return "SPIRV_UNEXPECTED_BLOCK_DATA";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE:     return "SPIRV_INVALID_BLOCK_MEMBER_REFERENCE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT:                return "SPIRV_INVALID_ENTRY_POINT";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE:             return "SPIRV_INVALID_EXECUTION_MODE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_MAX_RECURSIVE_EXCEEDED:             return "SPIRV_MAX_RECURSIVE_EXCEEDED";
        default:                                                                return "UNKNOWN";
    }
}

const char* get_descriptor_type_string(SpvReflectDescriptorType spv_type) {
    switch(spv_type) {
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:                       return "SAMPLER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:        return "COMBINED_IMAGE_SAMPLER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:                 return "SAMPLED_IMAGE";
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:                 return "STORAGE_IMAGE";
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:          return "UNIFORM_TEXEL_BUFFER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:          return "STORAGE_TEXEL_BUFFER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:                return "UNIFORM_BUFFER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:                return "STORAGE_BUFFER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:        return "UNIFORM_BUFFER_DYNAMIC";
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:        return "STORAGE_BUFFER_DYNAMIC";
        case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:              return "INPUT_ATTACHMENT";
        case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:    return "ACCELERATION_STRUCTURE_KHR";
        default:                                                        return "UNKNOWN";
    }
}

void get_shader_type(SpvReflectTypeDescription* spv_type) {   
    assert(spv_type != nullptr);
    ShaderType type;

    std::cout << "  type_flags = 0x" << std::hex << spv_type->type_flags << std::dec << '\n';
    const auto flags = spv_type->type_flags;

    if(flags & SPV_REFLECT_TYPE_FLAG_VOID) {
        type.base_type = ShaderBaseType::Void;
    }
    else if(flags & SPV_REFLECT_TYPE_FLAG_BOOL) {
        type.base_type = ShaderBaseType::Bool;
    }
    else if(flags & SPV_REFLECT_TYPE_FLAG_INT) {
        type.base_type = ShaderBaseType::Int;
    }
    else if(flags & SPV_REFLECT_TYPE_FLAG_FLOAT) {
        type.base_type = ShaderBaseType::Float;
    }
    else if(flags & SPV_REFLECT_TYPE_FLAG_VECTOR) {
        type.base_type = ShaderBaseType::Vector;
        type.vector_dim = spv_type->traits.numeric.vector.component_count;

        if(flags & SPV_REFLECT_TYPE_FLAG_FLOAT) {
            type.base_type = ShaderBaseType::Float;
        }
        else if(flags & SPV_REFLECT_TYPE_FLAG_INT) {
            type.base_type = ShaderBaseType::Int;
        }
        else if(flags & SPV_REFLECT_TYPE_FLAG_BOOL) {
            type.base_type = ShaderBaseType::Bool;
        }
        else assert(false);
    }
    else if(flags & SPV_REFLECT_TYPE_FLAG_MATRIX) {
        type.base_type = ShaderBaseType::Matrix;
        type.matrix_rows = spv_type->traits.numeric.matrix.row_count;
        type.matrix_cols = spv_type->traits.numeric.matrix.column_count;

    }
    else if(flags & SPV_REFLECT_TYPE_FLAG_ARRAY) {
        type.base_type = ShaderBaseType::Array;
        for(int i = 0; i < spv_type->traits.array.dims_count; i++) {

        }
    }
    else if(flags & SPV_REFLECT_TYPE_FLAG_STRUCT) {
        type.base_type = ShaderBaseType::Struct;

    }
    else assert(false);
}

ShaderInterfaceVariable get_interface_variable(SpvReflectInterfaceVariable* spv_input) {
    SpvReflectTypeDescription* spv_type = spv_input->type_description;
    // get_shader_type(spv_type);

    std::string name = spv_input->name ? spv_input->name : "<unnamed>";
    uint32_t location = spv_input->location;
    return ShaderInterfaceVariable(name, location);
}

ShaderResource get_resource(SpvReflectDescriptorBinding* spv_resource) {
    ShaderResourceType type;
    switch(spv_resource->descriptor_type) {
        
    }
    if(spv_resource->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER) {

    }

    std::string name = spv_resource->name;
    uint32_t set = spv_resource->set;
    uint32_t binding = spv_resource->binding;


    // std::cout
    //     << "Resource: " << (resource->name ? resource->name : "<unnamed>")
    //     << ", set = " << resource->set
    //     << ", binding = " << resource->binding
    //     << ", descriptor type = " << get_descriptor_type_string(resource->descriptor_type)
    //     << '\n';

    // for (uint32_t i = 0; i < resource->block.member_count; ++i) {
    //     auto& member = resource->block.members[i];
    //     std::cout
    //         << "    member: "
    //         << (member.name ? member.name : "<unnamed>")
    //         << ", offset = " << member.offset
    //         << ", size = " << member.size
    //         << '\n';
    // }
}

void try_with_error(SpvReflectResult spv_result, const char* err_msg) {
    if(spv_result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error(std::string(err_msg) + " : " + get_result_string(spv_result));
    }
}

ShaderInterface ShaderInterface::unify(ShaderInterface& left, ShaderInterface& right) {
    // make sure left outputs are compatible with right inputs 
    // TODO actually compare their types, for now just check that the locations match 
    std::set<uint32_t> left_locations, right_locations;
    for(ShaderInterfaceVariable& var : left.outputs) {
        assert(!left_locations.contains(var.location));
        left_locations.insert(var.location);
    }
    for(ShaderInterfaceVariable& var : right.inputs) {
        assert(!right_locations.contains(var.location));
        right_locations.insert(var.location);
    }
    if(left_locations != right_locations) {
        throw std::runtime_error("ShaderInterface unify left and right interfaces incompatible interface variables");
    }

    // make sure left and right resources don't conflict eachother
    // TODO

    // create new interface
    ShaderInterface ret;
    ret.inputs = left.inputs;
    ret.outputs = right.outputs;
    ret.resources.insert(ret.resources.end(), left.resources.begin(), left.resources.end());
    ret.resources.insert(ret.resources.end(), right.resources.begin(), right.resources.end());
    return ret;
}

Shader::Shader(Shader&& other) {
    stage = other.stage;
    source_code = std::move(other.source_code);
    spirv_code = std::move(other.spirv_code);
    interface = std::move(other.interface);
}

Shader::~Shader() = default;

Shader::Shader(
    ShaderStage stage, 
    std::string source_code, 
    std::vector<uint32_t> spirv_code
) : stage(stage), 
    source_code(source_code), 
    spirv_code(spirv_code) {
    introspect();
}

void Shader::introspect() {
    std::cout << "introspecting !!" << std::endl;
    std::cout << "SPIR-V code size : " << spirv_code.size() << std::endl;
    std::cout << "First uint32_t : 0x" << std::hex << spirv_code[0] << std::dec << std::endl;

    ShaderInterface interface;

    SpvReflectShaderModule spv_module{};
    try_with_error(
        spvReflectCreateShaderModule(
            spirv_code.size() * sizeof(uint32_t),
            spirv_code.data(),
            &spv_module
        ),
        "Failed to create SPIR-V reflection module"
    );
    std::cout << "Shader stage : " << get_stage_string(spv_module.shader_stage) << std::endl;

    // extract inputs
    {
        uint32_t count = 0;
        try_with_error(spvReflectEnumerateInputVariables(&spv_module, &count, NULL), "Failed to gather SPIR-V input variable count");
        std::vector<SpvReflectInterfaceVariable*> input_vars(count);
        try_with_error(spvReflectEnumerateInputVariables(&spv_module, &count, input_vars.data()), "Failed to gather SPIR-V input variables");
        for(auto* spv_input : input_vars) {
            if(spv_input->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) {
                continue;
            }
            ShaderInterfaceVariable var = get_interface_variable(spv_input);
            interface.inputs.push_back(var);
            std::cout << "Input : name = " << var.name << " location = " << var.location << std::endl;
        }
    }

    // extract outputs
    {
        uint32_t count = 0;
        try_with_error(spvReflectEnumerateOutputVariables(&spv_module, &count, nullptr), "Failed to gather SPIR-V output variable count");
        std::vector<SpvReflectInterfaceVariable*> outputs(count);
        try_with_error(spvReflectEnumerateOutputVariables(&spv_module, &count, outputs.data()), "Failed to gather SPIR-V output variables");

        for(auto* spv_output : outputs) {
            if(spv_output->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) {
                continue;
            }
            ShaderInterfaceVariable var = get_interface_variable(spv_output);
            interface.outputs.push_back(var);
            std::cout << "Output : name = " << var.name << " location = " << var.location << std::endl;
        }
    }

    // extract resources
    {
        uint32_t count = 0;
        try_with_error(spvReflectEnumerateDescriptorBindings(&spv_module, &count, nullptr), "Failed to gather SPIR-V resource count");
        std::vector<SpvReflectDescriptorBinding*> resources(count);
        try_with_error(spvReflectEnumerateDescriptorBindings(&spv_module, &count, resources.data()), "Failed to gather SPIR-V resources");

        for(auto* spv_resource : resources) {
            // ShaderResource res = get_resource(spv_resource);
            // interface.resources.push_back(res);
            // std::cout << "Resource : "
        }
    }

    spvReflectDestroyShaderModule(&spv_module);
}

}