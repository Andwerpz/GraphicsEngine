#pragma once
#include <vector>
#include <map>
#include <string>

#include <engine/util/error.h>

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

class Shader;
class Program;
class ShaderInterface {
    friend class Shader;
    friend class Program;

public:
	// will check that the left outputs match with the right inputs, and resources don't overlap 
	//   if a discrepancy is detected, should be a runtime error 
	// the same resource name on two different {set, binding} slots is an error
	// the generated shader interface will have the left inputs, right outputs, and both resources 
	static ShaderInterface unify(ShaderInterface& left, ShaderInterface& right);

    const ShaderResource& get_resource(ShaderResourceLocation location) const {
        if(!resources.contains(location)) {
            engine::util::throw_error("interface does not contain resource on location : {" + std::to_string(location.first) + ", " + std::to_string(location.second) + "}");
        }
        return resources.at(location);
    }
    const ShaderResourceType& get_resource_type(ShaderResourceLocation location) const {
        return get_resource(location).base_type;
    }
    ShaderResourceLocation get_resource_location(const std::string& name) const {
        auto found = resource_names.find(name);
        if(found == resource_names.end()) {
            engine::util::throw_error("interface does not contain resource : \"" + name + "\"");
        }
        return found->second;
    }

    const ShaderResource& get_resource(const std::string& name) const {
        return get_resource(get_resource_location(name));
    }
    const ShaderResourceType& get_resource_type(const std::string& name) const {
        return get_resource(name).base_type;
    }

    const ShaderInterfaceVariable& get_input(uint32_t location) const {
        if(!inputs.contains(location)) {
            engine::util::throw_error("interface does not contain input on location : " + std::to_string(location));
        }
        return inputs.at(location);
    }
    const ShaderInterfaceVariable& get_input(const std::string& name) const {
        return get_input(get_input_location(name));
    }
    uint32_t get_input_location(const std::string& name) const {
        auto found = input_names.find(name);
        if(found == input_names.end()) {
            engine::util::throw_error("interface does not contain input : \"" + name + "\"");
        }
        return found->second;
    }

    const ShaderInterfaceVariable& get_output(uint32_t location) const {
        if(!outputs.contains(location)) {
            engine::util::throw_error("interface does not contain output on location : " + std::to_string(location));
        }
        return outputs.at(location);
    }
    const ShaderInterfaceVariable& get_output(const std::string& name) const {
        return get_output(get_output_location(name));
    }
    uint32_t get_output_location(const std::string& name) const {
        auto found = output_names.find(name);
        if(found == output_names.end()) {
            engine::util::throw_error("interface does not contain output : \"" + name + "\"");
        }
        return found->second;
    }

    const std::map<uint32_t, ShaderInterfaceVariable>& get_inputs() const { return inputs; }
    const std::map<uint32_t, ShaderInterfaceVariable>& get_outputs() const { return outputs; }
    const std::map<ShaderResourceLocation, ShaderResource>& get_resources() const { return resources; }

private:
    ShaderInterface() = default;

    void add_input_variable(uint32_t location, ShaderInterfaceVariable variable);
    void add_output_variable(uint32_t location, ShaderInterfaceVariable variable);
    void add_resource(ShaderResourceLocation location, ShaderResource resource);

    // map location -> interface variable
    std::map<uint32_t, ShaderInterfaceVariable> inputs;
    std::map<uint32_t, ShaderInterfaceVariable> outputs;

    // map name -> location
    // names are unique within inputs, and unique within outputs
    std::map<std::string, uint32_t> input_names;
    std::map<std::string, uint32_t> output_names;

    // map {set, binding} -> resource
    std::map<ShaderResourceLocation, ShaderResource> resources;

    // map name -> {set, binding}
    // all resource names should be unique
    std::map<std::string, ShaderResourceLocation> resource_names;
};
    
}