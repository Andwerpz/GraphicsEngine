#pragma once
#include <cstdint>
#include <vector>
#include <optional>

namespace engine::graphics {

class Resource;
class Buffer;
class Texture;

// a ProgramInvocation is a container relating a Program and the resources required to run the program 
// to actually run a program, you need a ProgramInvocation

class ProgramInvocation {
protected:
    virtual ~ProgramInvocation() = 0;

	// bound resources to all shaders in this program
	struct ResourceBinding {
		Resource& resource;
		uint32_t set;
		uint32_t binding;
	};
	std::vector<ResourceBinding> resources;

public:
    // should make sure that the resources contained here can actually be used to invoke the given program
    // should only be able to invoke after validating 
    // once validated, you shouldn't be able to change any resource bindings
    void validate();

protected:
    virtual void validate_impl() = 0;
    bool validated = false;
};


// how to assign each element in a bound vertex buffer
enum class VertexInputRate {
    Vertex,		// every element goes to a different vertex
    Instance,	// every element goes to a different instance
};

// how each vertex attribute element should be interpreted
enum class VertexFormat {
    Float32x1,
    Float32x2,
    Float32x3,
    Float32x4,
};

// how each index element should be interpreted
enum class IndexFormat {
    UInt16,
    UInt32,
};

// everything required to invoke a rendering program
class RenderingProgramInvocation : public ProgramInvocation {
protected:
	// mapping a vertex buffer to an input interface variable
	struct VertexAttributeBinding {
		Buffer& buffer;         // what buffer to use 
		uint32_t location;      // what interface variable to bind to 
		uint32_t offset;        // byte of the first element
		uint32_t stride;        // bytes between the start of each element
		VertexInputRate rate;   // how to map elements
		VertexFormat format;    // how to interpret elements
	};
	std::vector<VertexAttributeBinding> attribute_bindings;

	// optionally bind an index buffer for indexed rendering
	struct IndexBufferBinding {
		Buffer& buffer;
		IndexFormat format;
	};
	std::optional<IndexBufferBinding> index_buffer;

    // mapping a texture to an output interface variable
    struct OutputBinding {
        uint32_t location;      // fragment shader output index
        Texture& texture;       // what texture to use 
    };
    std::vector<OutputBinding> output_bindings;

    // should be true if bind_window() is called
    // indicates this invocation should render directly to whatever window is managed by the graphics device
    // this being true should be mutually exclusive with any other texture being bound
    bool bound_window = false;

public:
    void bind_vertex_attribute(
        Buffer& buffer,
        uint32_t location,
        uint32_t offset,
        uint32_t stride,
        VertexInputRate rate,
        VertexFormat format
    );
    void bind_index_buffer(
        Buffer& buffer,
        IndexFormat format
    );
    void bind_output_texture(
        uint32_t location,
        Texture& texture
    );
    void bind_window();
	void draw(
        uint32_t nr_elements, 
        uint32_t nr_instances = 1
    );

	virtual void draw_impl(
        uint32_t nr_elements, 
        uint32_t nr_instances
    ) = 0;
};

class ComputeProgramInvocation : public ProgramInvocation {
    // TODO 
};

};
