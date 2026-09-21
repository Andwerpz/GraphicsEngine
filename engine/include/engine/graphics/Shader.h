#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <memory>

namespace engine::graphics {

// a Shader is some code that is meant to run on the GPU 
// a ShaderInterface describes the resources the Shader requires, and what slots they are required on 

// shader code should get compiled to SPIR-V so that it can get introspected 

enum class ShaderBaseType {
    Void,
    Bool,
    Int,
    Float,
    Struct,
    Matrix,
    Vector,
    Array,
};

struct ShaderType {
    ShaderBaseType base_type;
    uint32_t vector_dim = 0;
    uint32_t matrix_rows = 0;
    uint32_t matrix_cols = 0;
    std::vector<uint32_t> array_dims;
    std::vector<ShaderType> member_types;
};

struct ShaderInterfaceVariable {
    ShaderInterfaceVariable(std::string name, uint32_t location) : 
        name(name), location(location) {}

    // ShaderType type;
    std::string name;
    uint32_t location;
};

enum class ShaderResourceBaseType {
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

// if a resource has an identical ShaderResourceType to a ShaderResource, then it can bind there
struct ShaderResourceType {
    ShaderResourceBaseType base_type;

    // sampler
    ShaderSamplerDimension sampler_dim;
};

struct ShaderResource {
    ShaderResource(ShaderResourceType type, std::string name, uint32_t set, uint32_t binding) :
        type(type), name(name), set(set), binding(binding) {}

    ShaderResourceType type;
    std::string name;
    uint32_t set;
    uint32_t binding;
};

struct ShaderInterface {
    std::vector<ShaderInterfaceVariable> inputs;
    std::vector<ShaderInterfaceVariable> outputs;
    std::vector<ShaderResource> resources;

	// will check that the left outputs match with the right inputs, and resources don't overlap 
	//   if a discrepancy is detected, should be a runtime error 
	// the generated shader interface will have the left inputs, right outputs, and both resources 
	static ShaderInterface unify(ShaderInterface& left, ShaderInterface& right);
};

enum class ShaderStage {
    Vertex,
    Fragment,
    Compute,
};

class Shader {
public: 
    Shader(Shader&& other);
    virtual ~Shader() = 0;

protected:
    Shader(
        ShaderStage stage, 
        std::string source_code, 
        std::vector<uint32_t> spirv_code
    );

    // what sort of shader is this?
    ShaderStage stage;

    // original source code
    std::string source_code;

    // compiled SPIR-V code
    std::vector<uint32_t> spirv_code;

    // interface built by introspection
    ShaderInterface interface;

    // called as part of the constructor
    // constructs interface metadata for this shader 
    void introspect();

public:
    ShaderStage get_stage() {
        return stage;
    }

    std::string& get_source_code() {
        return source_code;
    }

    ShaderInterface& get_interface() {
        return interface;
    }
};

}
