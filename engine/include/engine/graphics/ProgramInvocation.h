#pragma once
#include <cstdint>
#include <vector>
#include <optional>
#include <map>
#include <memory>
#include <string>
#include <string_view>

#include <glm/glm.hpp>

#include "ShaderInterface.h"
#include "ShaderResource.h"

namespace engine::graphics {

class Resource;
class Buffer;
class Texture;
class Program;
class Sampler;
class GraphicsDevice;

// TODO need to handle non-buffer uniforms. These are probably more graphics API specific
//   seems like I may need to query non-buffer uniforms from the graphics API 
//   or I suppose I could directly parse the spirv code as well 
//   spirv-reflect doesn't seem to have a way to query these uniforms
// TODO instead of immediately updating uniform buffers, lazily update them  

// TODO maybe introduce the notion of a 'RenderTarget' 
//   currently, we have to handle rendering to the window as a special case
//   perhaps a RenderTarget could unify rendering to textures, or rendering directly to a window

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

    // map {set, binding} -> resource binding
    struct ResourceBinding {
        ShaderResourceType type;
        struct UniformBuffer {
            Buffer* buffer = nullptr;
        } uniform_buffer;
        struct SampledTexture {
            Texture* texture = nullptr;
            Sampler* sampler = nullptr;
        } sampled_texture;

        static ResourceBinding create_uniform_buffer(Buffer& buffer) {
            ResourceBinding res;
            res.type = ShaderResourceType::UniformBuffer;
            res.uniform_buffer.buffer = &buffer;
            return res;
        }
        static ResourceBinding create_sampled_texture(Texture& texture, Sampler& sampler) {
            ResourceBinding res;
            res.type = ShaderResourceType::SampledTexture;
            res.sampled_texture.texture = &texture;
            res.sampled_texture.sampler = &sampler;
            return res;
        }
    };
    std::map<ShaderResourceLocation, ResourceBinding> bound_resources;

    // uniform buffers managed by the invocation 
    // if a uniform buffer is not bound, the invocation will generate one
    std::vector<std::unique_ptr<Buffer>> uniform_buffers;

    // bind a resource to this program invocation
    // should be done before validation
    void bind_resource(ShaderResourceLocation location, ResourceBinding resource);

    // internal api to get some already bound resource binding
    ResourceBinding& get_resource(ShaderResourceLocation location);

public:
    // user facing api to bind resources
    // should correspond one-to-one with available ShaderResources 
    void bind_uniform_buffer(
        ShaderResourceLocation location, 
        Buffer& uniform_buffer
    );
    void bind_sampled_texture(
        ShaderResourceLocation location,
        Texture& texture, 
        Sampler& sampler
    );

    // user facing api to retrieve resources
    Buffer& get_uniform_buffer(ShaderResourceLocation location);

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
    void set_uniform(UniformToken<glm::mat4> token, glm::mat4 val);

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

    // should be true if bind_depth_buffer() is called
    // indicates that a depth buffer should be generated when validating
    bool bound_depth_buffer = false;

    // if true, enables depth testing
    // defaults to true if a depth buffer is bound or rendering to the window, false otherwise
    bool depth_testing = false;

    // if true, clears the color attachments at the beginning of execution
    bool clear_color = true;

    // if true, clears the depth buffer at the beginning of execution
    bool clear_depth = true;

	virtual void draw_impl(
        uint32_t nr_elements, 
        uint32_t nr_instances
    ) = 0;

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
    void bind_depth_buffer();

	void draw(
        uint32_t nr_elements, 
        uint32_t nr_instances = 1
    );

    // modifies the behaviour when executing the program
    // can be set at any time
    void do_depth_testing(bool val);
    void do_clear_color(bool val);
    void do_clear_depth(bool val);
};

class ComputeProgramInvocation : public ProgramInvocation {
    // TODO 
};

};
