#pragma once
#include <vector>
#include <map>

#include "ShaderResource.h"
#include "ShaderType.h"

namespace engine::graphics {

// a ShaderInterface describes the resources the Shader requires, and what slots they are required on 

struct ShaderInterfaceVariable {
    ShaderInterfaceVariable(ShaderType type, std::string name) : 
        type(type), name(name) {}
    ShaderInterfaceVariable(const ShaderInterfaceVariable& other) :
        type(other.type), name(other.name) {}

    ShaderType type;
    std::string name;
};

// {set, binding}
typedef std::pair<uint32_t, uint32_t> ShaderResourceLocation;

struct ShaderInterface {
    // map location -> interface variable
    std::map<uint32_t, ShaderInterfaceVariable> inputs;
    std::map<uint32_t, ShaderInterfaceVariable> outputs;

    // map {set, binding} -> resource
    std::map<ShaderResourceLocation, ShaderResource> resources;

	// will check that the left outputs match with the right inputs, and resources don't overlap 
	//   if a discrepancy is detected, should be a runtime error 
	// the generated shader interface will have the left inputs, right outputs, and both resources 
	static ShaderInterface unify(ShaderInterface& left, ShaderInterface& right);

    void add_input_variable(uint32_t location, ShaderInterfaceVariable variable);
    void add_output_variable(uint32_t location, ShaderInterfaceVariable variable);
    void add_resource(ShaderResourceLocation location, ShaderResource resource);
    
    ShaderResource& get_resource(ShaderResourceLocation location);
    ShaderResourceType get_resource_type(ShaderResourceLocation location);
};
    
}