#include <engine/graphics/ShaderType.h>
#include <engine/graphics/ShaderInterface.h>
#include <engine/graphics/ShaderResource.h>
#include <engine/graphics/Shader.h>
#include <engine/util/error.h>

#include <spirv_reflect.h>

#include <cassert>
#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <functional>

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

const char* get_image_dimension_string(SpvDim_ dim) {
    switch(dim) {
        case SpvDim1D:           return "1D";
        case SpvDim2D:           return "2D";
        case SpvDim3D:           return "3D";
        case SpvDimCube:         return "CUBE";
        case SpvDimRect:         return "RECT";
        case SpvDimBuffer:       return "BUFFER";
        case SpvDimSubpassData:  return "SUBPASS_DATA";
        default:                 return "UNKNOWN";
    }
}

struct SpirvInstruction {
    SpvOp opcode;
    std::vector<uint32_t> words;    // all words, including first word
};

// checks that the header is valid
// returns a list of SPIR-V instructions
std::vector<SpirvInstruction> parse_spirv_instructions(const std::vector<uint32_t>& spirv_code) {
    size_t ptr = 0;

    // parse header
    uint32_t id_bound;
    if(spirv_code.size() < 5) {
        engine::util::throw_error("Not enough words for header");
    }
    {
        // word 0: Magic number
        if(spirv_code[ptr ++] != 0x07230203) {
            engine::util::throw_error("Invalid magic number");
        }

        // word 1: Version
        ptr ++;

        // word 2: Generator magic number
        ptr ++;

        // word 3: ID Bound
        // SPIR-V guarantees that all IDs used are less than this bound
        id_bound = spirv_code[ptr ++];

        // word 4: Schema, this is currently reserved and must be 0
        if(spirv_code[ptr ++] != 0) {
            engine::util::throw_error("Expected schema to be 0");
        }
    }

    // parse instructions
    std::vector<SpirvInstruction> instructions;
    while(ptr < spirv_code.size()) {
        // first word contains (opcode | word_count << 16)
        // word_count is the length of the instruction including the first word
        SpvOp opcode = static_cast<SpvOp>(spirv_code[ptr] & 0xffff);
        uint32_t word_count = spirv_code[ptr] >> 16;
        if(word_count == 0) {
            engine::util::throw_error("word_count should be > 0");
        }
        if(ptr + word_count > spirv_code.size()) {
            engine::util::throw_error("invalid word_count");
        }

        // read in rest of words
        std::vector<uint32_t> words;
        for(uint32_t i = 0; i < word_count; i++) {
            words.push_back(spirv_code[ptr ++]);
        }
        instructions.push_back({opcode, words});
    }
    assert(ptr == spirv_code.size());
    return instructions;
}

std::string parse_spirv_string(const std::vector<uint32_t>& words, size_t start) {
    std::string ret = "";
    for(size_t i = start; i < words.size(); i++) {
        uint32_t word = words[i];
        for(int j = 0; j < 4; j++) {
            char c = (char) (word & 0xFF);
            word >>= 8;
            if(c == 0) return ret;
            ret.push_back(c);
        }
    }
    engine::util::throw_error("string does not have null terminator");
}

// returns a mapping from id -> ShaderType 
// documentation of "OpType*"" instructions : 
//   https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#OpTypeBool
std::map<uint32_t, ShaderType> parse_shader_types(const std::vector<SpirvInstruction>& instructions) {
    constexpr bool debug = false;
    auto get_from_id = []<typename T>(uint32_t id, std::map<uint32_t, T>& id_map) -> T& {
        if(!id_map.contains(id)) {
            engine::util::throw_error("Tried to get with undeclared id : " + std::to_string(id));
        }
        return id_map.at(id);
    };

    std::map<uint32_t, std::string> names;
    auto is_name_declared = [&names](uint32_t id) -> bool {
        return names.contains(id);
    };
    auto get_declared_name = [&get_from_id, &names](uint32_t id) -> std::string {
        return get_from_id(id, names);
    };

    std::map<std::pair<uint32_t, uint32_t>, std::string> struct_member_names;
    auto is_struct_member_name_declared = [&struct_member_names](uint32_t struct_id, uint32_t member_idx) -> bool {
        return struct_member_names.contains({struct_id, member_idx});
    };
    auto get_declared_struct_member_name = [&struct_member_names](uint32_t struct_id, uint32_t member_idx) -> std::string {
        if(!struct_member_names.contains({struct_id, member_idx})) {
            engine::util::throw_error("Tried to get undeclared struct member name : " + std::to_string(struct_id) + " " + std::to_string(member_idx));
        }
        return struct_member_names.at({struct_id, member_idx});
    };

    std::map<uint32_t, ShaderType> types;
    auto get_declared_type = [&get_from_id, &types](uint32_t id) -> ShaderType& {
        return get_from_id(id, types);
    };

    std::map<uint32_t, uint32_t> integer_constants;
    auto get_declared_integer_constant = [&get_from_id, &integer_constants](uint32_t id) -> uint32_t {
        return get_from_id(id, integer_constants);
    };

    std::map<uint32_t, float> float_constants;
    auto get_declared_float_constant = [&get_from_id, &float_constants](uint32_t id) -> float {
        return get_from_id(id, float_constants);
    };

    std::map<uint32_t, double> double_constants;
    auto get_declared_double_constant = [&get_from_id, &double_constants](uint32_t id) -> double {
        return get_from_id(id, double_constants);
    };

    for(size_t _ = 0; _ < instructions.size(); _++) {
        const SpirvInstruction& instruction = instructions[_];
        switch(instruction.opcode) {
            case SpvOpTypeVoid: {
                // OpTypeVoid <id (Result)>
                uint32_t id = instruction.words[1];
                ShaderType type((ShaderVoidType) {});
                types.emplace(id, type);
                if(debug) std::cout << "OpTypeVoid " << id << std::endl;
                break;
            }
            case SpvOpTypeBool: {
                // OpTypeBool <id (Result)>
                assert(instruction.words.size() == 2);
                uint32_t id = instruction.words[1];
                ShaderType type(ShaderScalarType::Bool);
                types.emplace(id, type);
                if(debug) std::cout << "OpTypeBool " << id << std::endl;
                break;
            }
            case SpvOpTypeInt: {
                // OpTypeInt <id (Result)> width signedness
                assert(instruction.words.size() == 4);
                uint32_t id = instruction.words[1];
                uint32_t width = instruction.words[2];
                uint32_t signedness = instruction.words[3];

                ShaderScalarType scalar_type;
                if(width != 32) {
                    engine::util::throw_error("Unsupported int bit width : " + std::to_string(width));
                }
                if(signedness) {
                    scalar_type = ShaderScalarType::Int;
                }
                else {
                    scalar_type = ShaderScalarType::UInt;
                }
                ShaderType type(scalar_type);
                types.emplace(id, type);

                if(debug) std::cout << "OpTypeInt " << id << std::endl;
                break;
            }
            case SpvOpTypeFloat: {
                // OpTypeFloat <id (Result)> width FP_encoding
                assert(instruction.words.size() >= 3);
                uint32_t id = instruction.words[1];
                uint32_t width = instruction.words[2];

                ShaderScalarType scalar_type;
                if(width == 32) {
                    scalar_type = ShaderScalarType::Float;
                }
                else if(width == 64) {
                    scalar_type = ShaderScalarType::Double;
                }
                else engine::util::throw_error("Unsupported float bit width : " + std::to_string(width));
                ShaderType type(scalar_type);
                types.emplace(id, type);

                if(debug) std::cout << "OpTypeFloat " << id << std::endl;
                break;
            }
            case SpvOpTypeVector: {
                // OpTypeVector <id (Result)> <id (Component Type)> component_count
                assert(instruction.words.size() == 4);
                uint32_t id = instruction.words[1];
                uint32_t base_type_id = instruction.words[2];
                uint32_t component_count = instruction.words[3];

                ShaderType& base_type = get_declared_type(base_type_id);
                assert(std::holds_alternative<ShaderScalarType>(base_type.type));
                ShaderVectorType vector_type(
                    std::get<ShaderScalarType>(base_type.type),
                    component_count
                );
                ShaderType type(vector_type);
                types.emplace(id, type);

                if(debug) std::cout << "OpTypeVector " << id << std::endl;
                break;
            }
            case SpvOpTypeMatrix: {
                // OpTypeMatrix <id (Result)> <id (Column Type)> column_count
                assert(instruction.words.size() == 4);
                uint32_t id = instruction.words[1];
                uint32_t base_type_id = instruction.words[2];
                uint32_t column_count = instruction.words[3];
                
                ShaderType& base_type = get_declared_type(base_type_id);
                assert(std::holds_alternative<ShaderVectorType>(base_type.type));
                ShaderVectorType& vector_type = std::get<ShaderVectorType>(base_type.type);
                ShaderMatrixType matrix_type(
                    vector_type.base_type,
                    vector_type.dimension,
                    column_count
                );
                ShaderType type(matrix_type);
                types.emplace(id, type);

                if(debug) std::cout << "OpTypeMatrix " << id << std::endl;
                break;
            }
            case SpvOpTypeImage: {
                // OpTypeImage <id (Result)> <id (Sampled Type)> dim depth arrayed multisampled sampled image_format access_qualifier
                // sampled_type refers to the type that results from sampling this image. Must be a scalar type or void
                // dim is the dimensionality of the image
                // depth indicates whether or not this is a depth image. 
                //   0 indicates no, 1 indicates yes, any other value is inconclusive
                // arrayed must be 0 or 1 and indicates whether or not the image has multiple layers
                // multisampled must be 0 or 1 and indicates whether or not the image has multisampled content
                // sampled indicates whether or not this image is accessed with a sampler. 
                //   0 indicates this is only known at runtime, 
                //   1 indicates an image that is sampled, 
                //   2 indicates an image that is compatible with read/write operations
                // image_format gives the format of each texel 
                // access_qualifier is optional and describes the access permissions of the image
                uint32_t id = instruction.words[1];
                uint32_t sampled_type_id = instruction.words[2];
                SpvDim_ dim = static_cast<SpvDim_>(instruction.words[3]);
                uint32_t depth = instruction.words[4];
                uint32_t arrayed = instruction.words[5];
                uint32_t multisampled = instruction.words[6];
                uint32_t sampled = instruction.words[7];
                uint32_t image_format = instruction.words[8];

                TextureDimension dimension;
                switch(dim) {
                    case SpvDim1D:
                        dimension = TextureDimension::D1;
                        break;
                    case SpvDim2D:
                        dimension = TextureDimension::D2;
                        break;
                    case SpvDim3D:
                        dimension = TextureDimension::D3;
                        break;
                    case SpvDimCube:
                        dimension = TextureDimension::Cube;
                        break;
                    default:
                        engine::util::throw_error("Unsupported SPIR-V texture dimension : " + std::string(get_image_dimension_string(dim)));
                }

                ShaderType& base_type = get_declared_type(sampled_type_id);
                assert(
                    std::holds_alternative<ShaderScalarType>(base_type.type) || 
                    std::holds_alternative<ShaderVoidType>(base_type.type)
                );
                std::variant<ShaderScalarType, ShaderVoidType> sampled_type = ShaderVoidType();
                if(std::holds_alternative<ShaderScalarType>(base_type.type)) {
                    sampled_type = std::get<ShaderScalarType>(base_type.type);
                }
                ShaderImageType image_type(
                    sampled_type,
                    dimension,
                    arrayed,
                    multisampled
                );
                ShaderType type(image_type);
                types.emplace(id, type);

                if(debug) std::cout << "OpTypeImage " << id << std::endl;
                break;
            }
            case SpvOpTypeSampler: {
                assert(false);
            }
            case SpvOpTypeSampledImage: {
                // OpTypeSampledImage <id (Result)> <id (Image Type)> 
                // image_type_id refers to a type declared by an OpTypeImage
                //   it must not have a dim of Buffer or SubpassData
                uint32_t id = instruction.words[1];
                uint32_t image_type_id = instruction.words[2];

                ShaderType& base_type = get_declared_type(image_type_id);
                assert(std::holds_alternative<ShaderImageType>(base_type.type));
                ShaderImageType& image_type = std::get<ShaderImageType>(base_type.type);
                ShaderSampledImageType sampled_image_type(
                    image_type  
                );
                ShaderType type(sampled_image_type);
                types.emplace(id, type);

                if(debug) std::cout << "OpTypeSampledImage " << id << std::endl;
                break;
            }
            case SpvOpTypeArray: {
                // OpTypeArray <id (Result)> <id (Element Type)> <id (Length)>
                uint32_t id = instruction.words[1];
                uint32_t base_type_id = instruction.words[2];
                uint32_t length_id = instruction.words[3];
                
                ShaderType& base_type = get_declared_type(base_type_id);
                uint32_t length = get_declared_integer_constant(length_id);
                ShaderArrayType array_type(
                    base_type,
                    length
                );
                ShaderType type(array_type);
                types.emplace(id, type);

                if(debug) std::cout << "OpTypeArray " << id << std::endl;
                break;
            }
            case SpvOpTypeRuntimeArray: {
                // OpTypeRuntimeArray <id (Result)> <id (Element Type)> 
                uint32_t id = instruction.words[1];
                uint32_t element_type_id = instruction.words[2];
                if(debug) std::cout << "OpTypeRuntimeArray " << id << std::endl;
                break;
            }
            case SpvOpTypeStruct: {
                // OpTypeStruct <id (Result)> <id (Member 0 Type)> <id (Member 1 Type)> ... 
                uint32_t id = instruction.words[1];

                std::vector<ShaderType> member_types;
                for(size_t i = 2; i < instruction.words.size(); i++) {
                    uint32_t member_id = instruction.words[i];
                    ShaderType& member_type = get_declared_type(member_id);
                    member_types.emplace_back(member_type);
                }
                ShaderStructType struct_type(member_types);

                // retrieve names if they exist
                if(is_name_declared(id)) {
                    struct_type.name = get_declared_name(id);
                    if(debug) std::cout << "Struct name : " << struct_type.name << std::endl;
                }
                for(size_t i = 0; i < struct_type.members.size(); i++) {
                    if(is_struct_member_name_declared(id, i)) {
                        struct_type.members[i].name = get_declared_struct_member_name(id, i);
                        if(debug) std::cout << "    Struct member name : " << struct_type.members[i].name << std::endl;
                    }
                }

                ShaderType type(struct_type);
                types.emplace(id, type);

                if(debug) std::cout << "OpTypeStruct " << id << std::endl;
                break;
            }
            case SpvOpTypeOpaque: {
                assert(false);
            }
            case SpvOpTypePointer: {
                // OpTypePointer <id (Result)> storage_class <id (Type)> 
                uint32_t id = instruction.words[1];
                uint32_t storage_class = instruction.words[2];
                uint32_t base_type_id = instruction.words[3];

                ShaderType& base_type = get_declared_type(base_type_id);
                ShaderPointerType pointer_type(base_type);
                ShaderType type(pointer_type);
                types.emplace(id, type);

                if(debug) std::cout << "OpTypePointer " << id << std::endl;
                break;
            }
            case SpvOpTypeFunction: {
                // OpTypeFunction <id (Result)> <id (Return Type)> <id (Parameter 0 Type)> ... 
                uint32_t id = instruction.words[1];
                uint32_t return_type_id = instruction.words[2];

                ShaderType& return_type = get_declared_type(return_type_id);
                std::vector<ShaderType> parameter_types;
                for(size_t i = 3; i < instruction.words.size(); i++) {
                    uint32_t parameter_id = instruction.words[i];
                    ShaderType& parameter_type = get_declared_type(parameter_id);
                    parameter_types.emplace_back(parameter_type);
                }
                ShaderFunctionType function_type(return_type, parameter_types);
                ShaderType type(function_type);
                types.emplace(id, type);

                if(debug) std::cout << "OpTypeFunction " << id << std::endl;
                break;
            }


            case SpvOpConstant: {
                // OpConstant <id (Type)> <id (Result)> literal
                uint32_t type_id = instruction.words[1];
                uint32_t id = instruction.words[2];

                ShaderType& type = get_declared_type(type_id);
                assert(std::holds_alternative<ShaderScalarType>(type.type));
                ShaderScalarType scalar_type = std::get<ShaderScalarType>(type.type);
                switch(scalar_type) {
                    case ShaderScalarType::Int:
                    case ShaderScalarType::UInt: {
                        assert(instruction.words.size() == 4);
                        uint32_t val = instruction.words[3];
                        integer_constants.insert({id, val});
                        break;
                    }
                    case ShaderScalarType::Float: {
                        assert(instruction.words.size() == 4);
                        float val = std::bit_cast<float>(instruction.words[3]);
                        float_constants.insert({id, val});
                        break;
                    }
                    case ShaderScalarType::Double: {
                        assert(instruction.words.size() == 5);
                        uint64_t bits = static_cast<uint64_t>(instruction.words[3]) | (static_cast<uint64_t>(instruction.words[4]) << 32);
                        double val = std::bit_cast<double>(bits);
                        double_constants.insert({id, val});
                        break;
                    }
                    default:
                        assert(false);
                }

                if(debug) std::cout << "OpConstant " << id << std::endl;
                break;
            }
            case SpvOpName: {
                // OpName <id (Target)> name
                uint32_t target_id = instruction.words[1];
                std::string name = parse_spirv_string(instruction.words, 2);
                names.emplace(target_id, name);
                if(debug) std::cout << "OpName " << target_id << " " << name << std::endl;
                break;
            }
            case SpvOpMemberName: {
                // OpMemberName <id (OpTypeStruct instruction)> member name
                uint32_t target_id = instruction.words[1];
                uint32_t member_ind = instruction.words[2];
                std::string name = parse_spirv_string(instruction.words, 3);
                struct_member_names.emplace(std::make_pair(target_id, member_ind), name);
                if(debug) std::cout << "OpMemberName " << target_id << std::endl;
                break;
            }
            default:
                // skip this instruction
                if(debug) std::cout << "Skipping instruction : " << instruction.opcode << std::endl;
                break;
        }
    }
    return types;
}

ShaderBlockVariable get_block_variable(SpvReflectBlockVariable* spv_blockvar) {
    ShaderBlockVariable ret;
    ret.name = spv_blockvar->name;
    ret.offset = spv_blockvar->offset;
    ret.size = spv_blockvar->size;

    // struct info
    if(spv_blockvar->member_count > 0) {
        ret.struct_info.members.reserve(spv_blockvar->member_count);
        for(uint32_t i = 0; i < spv_blockvar->member_count; ++i) {
            ret.struct_info.members.push_back(get_block_variable(&spv_blockvar->members[i]));
        }
    }

    // array info
    if(spv_blockvar->array.dims_count > 0) {
        ret.array_info.stride = spv_blockvar->array.stride;
        ret.array_info.dimensions.reserve(spv_blockvar->array.dims_count);
        for(uint32_t i = 0; i < spv_blockvar->array.dims_count; ++i) {
            ret.array_info.dimensions.push_back(spv_blockvar->array.dims[i]);
        }
    }

    return ret;
};

void print_block_variable(const ShaderBlockVariable& block, uint32_t indent = 0) {
    const std::string indentation(indent * 4, ' ');
    printf(
        "%sname: %s, offset: %llu, size: %llu\n",
        indentation.c_str(),
        block.name.c_str(),
        static_cast<unsigned long long>(block.offset),
        static_cast<unsigned long long>(block.size)
    );
    if(!block.array_info.dimensions.empty()) {
        printf(
            "    %sarray stride: %llu\n",
            indentation.c_str(),
            static_cast<unsigned long long>(block.array_info.stride)
        );
        for(uint64_t i = 0; i < block.array_info.dimensions.size(); ++i) {
            printf(
                "    %sdimension %llu: %llu\n",
                indentation.c_str(),
                static_cast<unsigned long long>(i),
                static_cast<unsigned long long>(
                    block.array_info.dimensions[i]
                )
            );
        }
    }
    if(!block.struct_info.members.empty()) {
        printf(
            "    %smembers: %zu\n",
            indentation.c_str(),
            block.struct_info.members.size()
        );
        for(const ShaderBlockVariable& member : block.struct_info.members) {
            print_block_variable(member, indent + 1);
        }
    }
}

void try_with_error(SpvReflectResult spv_result, const char* err_msg) {
    if(spv_result != SPV_REFLECT_RESULT_SUCCESS) {
        engine::util::throw_error(std::string(err_msg) + " : " + get_result_string(spv_result));
    }
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
    constexpr bool debug = true;
    if(debug) {
        std::cout << "introspecting !!" << std::endl;
        std::cout << "SPIR-V code size : " << spirv_code.size() << std::endl;
        std::cout << "First uint32_t : 0x" << std::hex << spirv_code[0] << std::dec << std::endl;
    }

    // SPIR-V reflect
    SpvReflectShaderModule spv_module{};
    try_with_error(
        spvReflectCreateShaderModule(
            spirv_code.size() * sizeof(uint32_t),
            spirv_code.data(),
            &spv_module
        ),
        "Failed to create SPIR-V reflection module"
    );
    if(debug) std::cout << "Shader stage : " << get_stage_string(spv_module.shader_stage) << std::endl;

    // custom reflect
    std::vector<SpirvInstruction> spirv_instructions = parse_spirv_instructions(spirv_code);
    std::map<uint32_t, ShaderType> shader_type_map = parse_shader_types(spirv_instructions);

    auto get_shader_type = [&shader_type_map](SpvReflectTypeDescription* spv_type) -> ShaderType {
        return shader_type_map.at(spv_type->id);
    };

    auto get_interface_variable = [&shader_type_map](SpvReflectInterfaceVariable* spv_var, uint32_t& location) -> ShaderInterfaceVariable {
        SpvReflectTypeDescription* spv_type = spv_var->type_description;
        ShaderType type = shader_type_map.at(spv_type->id);
        std::string name = spv_var->name ? spv_var->name : "<unnamed>";
        location = spv_var->location;
        return ShaderInterfaceVariable(type, name);
    };

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
            uint32_t location;
            ShaderInterfaceVariable var = get_interface_variable(spv_input, location);
            interface.add_input_variable(location, var);
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
            uint32_t location;
            ShaderInterfaceVariable var = get_interface_variable(spv_output, location);
            interface.add_output_variable(location, var);
        }
    }

    // extract resources
    {
        uint32_t count = 0;
        try_with_error(spvReflectEnumerateDescriptorBindings(&spv_module, &count, nullptr), "Failed to gather SPIR-V resource count");
        std::vector<SpvReflectDescriptorBinding*> resources(count);
        try_with_error(spvReflectEnumerateDescriptorBindings(&spv_module, &count, resources.data()), "Failed to gather SPIR-V resources");

        for(auto* spv_resource : resources) {
            std::string name = spv_resource->name;
            uint32_t set = spv_resource->set;
            uint32_t binding = spv_resource->binding;
            switch(spv_resource->descriptor_type) {
                case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
                    // requires a buffer
                    ShaderType type = get_shader_type(spv_resource->type_description);
                    ShaderBlockVariable blockvar = get_block_variable(&spv_resource->block);
                    ShaderResource resource = ShaderResource::create_uniform_buffer(name, type, blockvar);
                    interface.add_resource({set, binding}, resource);
                    break;
                }
                case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
                    // an image and a sampler combined into one
                    ShaderType type = get_shader_type(spv_resource->type_description);
                    assert(std::holds_alternative<ShaderSampledImageType>(type.type));
                    ShaderImageType& image_type = std::get<ShaderSampledImageType>(type.type).image_type;
                    ShaderResource resource = ShaderResource::create_sampled_texture(name, image_type);
                    interface.add_resource({set, binding}, resource);
                    break;
                }
                default:
                    std::cout << "Unsupported resource type : " << get_descriptor_type_string(spv_resource->descriptor_type) << std::endl;
                    assert(false);
            }
        }
    }

    spvReflectDestroyShaderModule(&spv_module);

    if(debug) {
        for(auto i = interface.inputs.begin(); i != interface.inputs.end(); i++) {
            ShaderInterfaceVariable& var = i->second;
            std::cout << "Input : " << std::to_string(i->first) << "\n";
            std::cout << "    name = " << var.name << "\n";
            std::cout << "    type = " << to_string(var.type) << "\n";
        }
        for(auto i = interface.outputs.begin(); i != interface.outputs.end(); i++) {
            ShaderInterfaceVariable& var = i->second;
            std::cout << "Output : " << std::to_string(i->first) << "\n";
            std::cout << "    name = " << var.name << "\n";
            std::cout << "    type = " << to_string(var.type) << "\n";
        }
        for(auto i = interface.resources.begin(); i != interface.resources.end(); i++) {
            ShaderResource resource = i->second;
            uint32_t set = i->first.first;
            uint32_t binding = i->first.second;
            std::cout << "Resource : " << "\n";
            std::cout << "    set = " << set << "\n";
            std::cout << "    binding = " << binding << "\n";
            switch(resource.base_type) {
                case ShaderResourceType::UniformBuffer: {
                    std::cout << "    resource_type = UniformBuffer" << "\n";
                    std::cout << "    type = " << to_string(resource.uniform_buffer.type) << "\n";
                    std::cout << "    blockvar :\n";
                    print_block_variable(resource.uniform_buffer.blockvar, 2);
                    break;
                }
                case ShaderResourceType::SampledTexture: {
                    std::cout << "    resource_type = SampledTexture" << "\n";
                    std::cout << "    image_type = " << to_string(resource.sampled_texture.image_type) << "\n";
                    break;
                }
                default:
                    assert(false);
            }
        }
    }
}

}