#include <engine/graphics/ShaderInterface.h>
#include <engine/util/error.h>

#include <iostream>

namespace engine::graphics {

ShaderInterface ShaderInterface::unify(ShaderInterface& left, ShaderInterface& right) {
    ShaderInterface ret;

    // make sure left outputs are compatible with right inputs 
    if(left.outputs.size() != right.inputs.size()) {
        engine::util::throw_error(
            "left and right do not have same amount of interface variables, " + 
            std::to_string(left.outputs.size()) + " vs. " + std::to_string(right.inputs.size())
        );
    }
    for(auto i = left.outputs.begin(); i != left.outputs.end(); i++) {
        uint32_t location = i->first;
        if(!right.inputs.contains(location)) {
            engine::util::throw_error("left has variable on location " + std::to_string(location) + " while right does not");
        }
        ShaderInterfaceVariable& left_var = i->second;
        ShaderInterfaceVariable& right_var = right.inputs.at(location);
        if(!(left_var.type == right_var.type)) {
            engine::util::throw_error(
                "left and right interface variable types are not equal at location " + std::to_string(location) +
                "left : " + to_string(left_var.type) + " right : " + to_string(right_var.type)
            );
        }
    }

    // add inputs and outputs
    ret.inputs = left.inputs;
    ret.input_names = left.input_names;
    ret.outputs = right.outputs;
    ret.output_names = right.output_names;

    // add left resources
    ret.resources = left.resources;
    ret.resource_names = left.resource_names;

    // add right resources
    for(auto i = right.resources.begin(); i != right.resources.end(); i++) {
        std::pair<uint32_t, uint32_t> location = i->first;
        ShaderResource& res = i->second;
        if(ret.resources.contains(location)) {
            // make sure the resource is compatible with this one
            ShaderResource& curr_res = ret.resources.at(location);
            if(!(res == curr_res)) {
                engine::util::throw_error("Incompatible resource");
            }
        }
        else {
            // make sure the resource name isn't already used
            if(ret.resource_names.contains(res.name)) {
                engine::util::throw_error("Duplicate resource name : " + res.name);
            }

            // insert the resource
            ret.resources.insert({location, i->second});
            ret.resource_names.insert({res.name, location});
        }
    }
    
    return ret;
}

void ShaderInterface::add_input_variable(uint32_t location, ShaderInterfaceVariable variable) {
    if(inputs.contains(location)) {
        engine::util::throw_error("interface already has input on location : " + std::to_string(location));
    }
    if(input_names.contains(variable.name)) {
        engine::util::throw_error("duplicate input variable name : " + variable.name);
    }
    inputs.insert({location, variable});
    input_names.insert({variable.name, location});
}

void ShaderInterface::add_output_variable(uint32_t location, ShaderInterfaceVariable variable) {
    if(outputs.contains(location)) {
        engine::util::throw_error("interface already has output on location : " + std::to_string(location));
    }
    if(output_names.contains(variable.name)) {
        engine::util::throw_error("duplicate output variable name : " + variable.name);
    }
    outputs.insert({location, variable});
    output_names.insert({variable.name, location});
}

void ShaderInterface::add_resource(ShaderResourceLocation location, ShaderResource resource) {
    if(resources.contains(location)) {
        engine::util::throw_error("interface already contains resource on location : {" + std::to_string(location.first) + ", " + std::to_string(location.second) + "}");
    }
    if(resource_names.contains(resource.name)) {
        engine::util::throw_error("duplicate resource name : " + resource.name);
    }
    resources.insert({location, resource});
    resource_names.insert({resource.name, location});
}

}