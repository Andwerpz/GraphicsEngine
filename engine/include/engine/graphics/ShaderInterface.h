#pragma once
#include <vector>
#include <map>

#include "ShaderResource.h"
#include "ShaderType.h"

namespace engine::graphics {

// a ShaderInterface describes the resources the Shader requires, and what slots they are required on 

struct ShaderInterfaceVariable {
    ShaderInterfaceVariable(ShaderType type, std::string name, uint32_t nr_locations) : 
        type(type), name(name), nr_locations(nr_locations) {}
    ShaderInterfaceVariable(const ShaderInterfaceVariable& other) :
        type(other.type), name(other.name), nr_locations(other.nr_locations) {}

    ShaderType type;
    std::string name;

    // the number of locations this interface variable takes up
    // when you see something like "layout (location = 0) in mat4 frag_pos;"
    //   this interface variable starts at location 0 and takes up 4 locations
    // the number of locations an interface variable takes up is standard
    //   and should not change depending on the specific driver you are compiling for
    uint32_t nr_locations;
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