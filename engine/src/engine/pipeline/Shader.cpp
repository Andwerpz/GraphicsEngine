#include <engine/pipeline/Shader.h>
#include <engine/pipeline/opengl/Shader.h>
#include <engine/graphics/opengl/OpenGLGraphicsDevice.h>

#include <spirv_reflect.h>

#include <cassert>
#include <iostream>
#include <fstream>

namespace engine::pipeline {

const char* get_stage_string(SpvReflectShaderStageFlagBits stage) {
    switch (stage) {
        case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:                   return "VERTEX";
        case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT:     return "TESSELLATION_CONTROL";
        case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:  return "TESSELLATION_EVALUATION";
        case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:                 return "GEOMETRY";
        case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:                 return "FRAGMENT";
        case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:                  return "COMPUTE";
        default:                                                    return "UNKNOWN";
    }
}

const char* get_result_string(SpvReflectResult result) {
    switch(result) {
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

void try_with_error(SpvReflectResult result, const char* err_msg) {
    if(result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error(std::string(err_msg) + " : " + get_result_string(result));
    }
}

void Shader::introspect() {
    std::cout << "introspecting !!" << std::endl;
    std::cout << "SPIR-V code size : " << spirv_code.size() << std::endl;
    std::cout << "First uint32_t : 0x" << std::hex << spirv_code[0] << std::dec << std::endl;

    SpvReflectShaderModule module{};
    try_with_error(
        spvReflectCreateShaderModule(
            spirv_code.size() * sizeof(uint32_t),
            spirv_code.data(),
            &module
        ),
        "Failed to create SPIR-V reflection module"
    );
    std::cout << "Shader stage : " << get_stage_string(module.shader_stage) << std::endl;

    // extract inputs
    {
        uint32_t count = 0;
        try_with_error(spvReflectEnumerateInputVariables(&module, &count, NULL), "Failed to gather SPIR-V input variable count");
        std::vector<SpvReflectInterfaceVariable*> input_vars(count);
        try_with_error(spvReflectEnumerateInputVariables(&module, &count, input_vars.data()), "Failed to gather SPIR-V input variables");

        for(auto* input : input_vars) {
            if(input->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) {
                continue;
            }
            std::cout << "Input: " << (input->name ? input->name : "<unnamed>") << ", location = " << input->location << std::endl;
        }
    }

    // extract outputs
    {
        uint32_t count = 0;
        try_with_error(spvReflectEnumerateOutputVariables(&module, &count, nullptr), "Failed to gather SPIR-V output variable count");
        std::vector<SpvReflectInterfaceVariable*> outputs(count);
        try_with_error(spvReflectEnumerateOutputVariables(&module, &count, outputs.data()), "Failed to gather SPIR-V output variables");

        for(auto* output : outputs) {
            if(output->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) {
                continue;
            }
            std::cout << "Output: " << (output->name ? output->name : "<unnamed>") << ", location = " << output->location << std::endl;
        }
    }

    // extract uniforms
    {

    }


    spvReflectDestroyShaderModule(&module);
}

}