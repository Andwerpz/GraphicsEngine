#include <engine/graphics/ProgramInvocation.h>
#include <cassert>
#include <stdexcept>

namespace engine::graphics {

ProgramInvocation::~ProgramInvocation() = default;

void ProgramInvocation::validate() {
    assert(!validated);
    validate_impl();
    validated = true;
}


void RenderingProgramInvocation::bind_vertex_attribute(
    Buffer& buffer,
    uint32_t location,
    uint32_t offset,
    uint32_t stride,
    VertexInputRate rate,
    VertexFormat format
) {
    // make sure we haven't validated
    assert(!validated);

    // make sure that this location hasn't been bound to yet
    for(VertexAttributeBinding& binding : attribute_bindings) {
        if(binding.location == location) {
            throw std::runtime_error("bind_vertex_attribute location already bound");
        }
    }

    // add binding
    attribute_bindings.emplace_back((VertexAttributeBinding) {
        buffer,
        location,
        offset,
        stride,
        rate,
        format
    });
}

void RenderingProgramInvocation::bind_index_buffer(
    Buffer& buffer,
    IndexFormat format
) {
    // make sure we haven't validated
    assert(!validated);
    
    // make sure we haven't bound an index buffer already
    if(index_buffer.has_value()) {
        throw std::runtime_error("bind_index_buffer index buffer already bound");
    }

    // add binding
    index_buffer.emplace((IndexBufferBinding) {
        buffer,
        format
    });
}

void RenderingProgramInvocation::bind_output_texture(
    uint32_t location,
    Texture& texture
) {
    // make sure we haven't validated
    assert(!validated);

    // make sure we haven't bound the window
    assert(!bound_window);

    // make sure we haven't bound a texture to this location
    for(OutputBinding& binding : output_bindings) {
        if(binding.location == location) {
            throw std::runtime_error("bind_output_texture location already bound");
        }
    }

    // add binding
    output_bindings.emplace_back((OutputBinding) {
        location, texture
    });
}

void RenderingProgramInvocation::bind_window() {
    // make sure we haven't validated
    assert(!validated);

    // make sure that we haven't bound any textures
    assert(output_bindings.size() == 0);
    
    bound_window = true;
}

void RenderingProgramInvocation::draw(
    uint32_t nr_elements, 
    uint32_t nr_instances
) {
    // make sure we have validated
    assert(validated);

    // call subclass impl
    draw_impl(
        nr_elements,
        nr_instances
    );
}

}