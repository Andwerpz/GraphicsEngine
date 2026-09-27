#include <engine/graphics/Program.h>
#include <engine/graphics/Shader.h>
#include <cassert>
#include <math.h>

namespace engine::graphics {

Program::Program(Program&& other) {
    interface = std::move(other.interface);
}

Program::Program(std::vector<Shader*> shaders) {
    assert(shaders.size() > 0);
    for(Shader* s : shaders) {
        assert(s != nullptr);
    }

    // generate interface
    interface = shaders[0]->get_interface();
    for(int i = 1; i < shaders.size(); i++) {
        interface = ShaderInterface::unify(interface, shaders[i]->get_interface());
    }
}

Program::~Program() = default;

uint32_t Program::get_output_location_extent() {
    if(interface.outputs.size() == 0) return 0;
    return interface.outputs.rbegin()->first + 1;
}


RenderingProgram::RenderingProgram(RenderingProgram&& other) : Program(std::move(other)) {
    primitive_topology = other.primitive_topology;
}

RenderingProgram::RenderingProgram(
    Shader& vertex,
    Shader& fragment,
    PrimitiveTopology _primitive_topology
) : Program({&vertex, &fragment}) {
    primitive_topology = _primitive_topology;
}


ComputeProgram::ComputeProgram(ComputeProgram&& other) : Program(std::move(other)) {
    // do nothing
}

ComputeProgram::ComputeProgram(Shader& compute) : Program({&compute}) {
    // do nothing
}

}