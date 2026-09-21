#pragma once
#include <memory>
#include "Shader.h"

namespace engine::graphics {

class RenderingProgramInvocation;
class ComputeProgramInvocation;

// a Program is a container for some executable GPU code

class Program {
public:
	Program(Program&&);
	virtual ~Program() = 0;

protected:
	// checks that the provided sequence of shaders actually form a valid program
	Program(std::vector<Shader*> shaders);

	// unified interface of all shaders of this program 
	ShaderInterface interface;

public:
	ShaderInterface& get_interface() {
		return interface;
	}

	// goes through all outputs of this program interface and returns the maximum location + 1
	// if there are no outputs, returns 0
	uint32_t get_output_location_extent();
};


// describes how vertices should be grouped to be rendered
enum class PrimitiveTopology {
	// every vertex is a point
	PointList,

	// every group of two vertices is an independent line segment
	LineList,

	// every group of three vertices is an independent triangle
	TriangleList,
};

class RenderingProgram : public Program {
public:
	RenderingProgram(RenderingProgram&&);

protected:
	RenderingProgram(
		Shader& vertex, 
		Shader& fragment, 
		PrimitiveTopology primitive_topology
	);

	// the sort of primitives that this rendering program expects
	PrimitiveTopology primitive_topology;

public:
    PrimitiveTopology get_primitive_topology() const {
        return primitive_topology;
    }

	// generate invocation 
	virtual std::unique_ptr<RenderingProgramInvocation> create_invocation() = 0;
};


class ComputeProgram : public Program {
public:
	ComputeProgram(ComputeProgram&&);

protected:
	ComputeProgram(Shader& compute);

public:
	// generate invocation
	virtual std::unique_ptr<ComputeProgramInvocation> create_invocation() = 0;
};

}
