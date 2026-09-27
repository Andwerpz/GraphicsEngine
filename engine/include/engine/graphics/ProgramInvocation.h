#pragma once
#include <cstdint>
#include <vector>
#include <optional>
#include <map>
#include <memory>
#include <string>
#include <string_view>

#include <glm/glm.hpp>

namespace engine::graphics {

class Resource;
class Buffer;
class Texture;
class Program;
class GraphicsDevice;

// TODO need to handle non-buffer uniforms. These are probably more graphics API specific
//   seems like I may need to query non-buffer uniforms from the graphics API 
//   or I suppose I could directly parse the spirv code as well 
//   spirv-reflect doesn't seem to have a way to query these uniforms
// TODO instead of immediately updating uniform buffers, lazily update them  

// resolved handle to a location in a uniform buffer 
// tokens are templated in order to provide compile-time checks that your uniform types are correct
template<typename T>
struct UniformToken {
    // byte offset into uniform buffer
    uint64_t offset;

    // uniform buffer resource binding location
    std::pair<uint32_t, uint32_t> location;
};

// a ProgramInvocation is a container relating a Program and the resources required to run the program 
// to actually run a program, you need a ProgramInvocation
class ProgramInvocation {
protected:
    ProgramInvocation(
        GraphicsDevice& device,
        Program& program
    );
    virtual ~ProgramInvocation() = 0;

    // underlying graphics API
    GraphicsDevice& device;

    // underlying program
    Program& program;

    // map {set, binding} -> resource
    std::map<std::pair<uint32_t, uint32_t>, std::reference_wrapper<Resource>> resources;

    // uniform buffers managed by the invocation 
    // if a uniform buffer is not bound, the invocation will generate one
    std::vector<std::unique_ptr<Buffer>> uniform_buffers;

public:
    // bind a resource to this program invocation
    // should be done before validation
    void bind_resource(uint32_t set, uint32_t binding, Resource& resource);
    void bind_resource(std::pair<uint32_t, uint32_t> location, Resource& resource);

    Resource& get_resource(uint32_t set, uint32_t binding);
    Resource& get_resource(std::pair<uint32_t, uint32_t> location);

    // should make sure that the resources contained here can actually be used to invoke the given program
    // should only be able to invoke the program after validating 
    // once validated, you shouldn't be able to change any resource bindings
    void validate();

    // retrieve a token that can be used to set uniforms
    // meant to be done once at init-time as parsing the address string is slow
    template<typename T>
    UniformToken<T> get_uniform(std::string_view addr);

    // use generated tokens to set uniform values 
    void set_uniform(UniformToken<float> token, float val);
    void set_uniform(UniformToken<glm::vec3> token, glm::vec3 val);

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
    RenderingProgramInvocation(
        GraphicsDevice& device,
        Program& program
    );

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
