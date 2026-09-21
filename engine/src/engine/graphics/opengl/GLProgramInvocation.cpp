#include <engine/graphics/opengl/GLProgramInvocation.h>
#include <engine/graphics/opengl/GLGraphicsDevice.h>

namespace engine::graphics::opengl {

GLRenderingProgramInvocation::GLRenderingProgramInvocation(
    GLGraphicsDevice& device,
    GLRenderingProgram& program
) : device(device),
    program(program) {
    vertex_array = std::nullopt;
}

void GLRenderingProgramInvocation::validate_impl() {
    // make sure that all inputs are bound

    // make sure all resources are bound

    // make sure no two outputs are bound to the same texture
    
    // make sure no two textures are bound to the same output

    // create vertex array 
    vertex_array.emplace(device.gl_create_vertex_array());
    for(VertexAttributeBinding& binding : attribute_bindings) {
        GLBuffer& gl_buffer = dynamic_cast<GLBuffer&>(binding.buffer);
        vertex_array.value().gl_bind_vertex_buffer(
            binding.location,
            gl_buffer,
            binding.offset,
            binding.stride
        );
    }
    if(index_buffer.has_value()) {
        GLBuffer& gl_buffer = dynamic_cast<GLBuffer&>(index_buffer.value().buffer);
        vertex_array.value().gl_bind_index_buffer(
            gl_buffer,
            index_buffer.value().format
        );
    }

    // create framebuffer
    if(bound_window) {
        framebuffer.emplace(device.gl_get_window_framebuffer());
    }
    else {
        framebuffer.emplace(device.gl_create_framebuffer());
        std::vector<GLenum> output_mapping(program.get_output_location_extent(), GL_NONE);
        uint32_t color_attachment_ind = 0;
        for(OutputBinding& binding : output_bindings) {
            GLTexture& gl_texture = dynamic_cast<GLTexture&>(binding.texture);
            framebuffer.value().gl_bind_color_attachment(
                color_attachment_ind,
                gl_texture
            );
            output_mapping[binding.location] = GL_COLOR_ATTACHMENT0 + color_attachment_ind;
        }
        framebuffer.value().gl_set_draw_buffers(output_mapping);
        if(!framebuffer.value().gl_is_complete()) {
            throw std::runtime_error("validate_impl failed to create framebuffer");
        }
    }
}

void GLRenderingProgramInvocation::draw_impl(
    uint32_t nr_vertices, 
    uint32_t nr_instances
) {
    // start execution context
    device.gl_begin();

    // bind vertex array
    assert(vertex_array.has_value());
    device.gl_bind_vertex_array(vertex_array.value());

    // bind framebuffer
    assert(framebuffer.has_value());
    device.gl_bind_framebuffer(framebuffer.value());

    // execute program
    device.gl_bind_rendering_program(program);
    if(vertex_array.value().has_index_buffer()) {
        if(nr_instances == 1) {
            device.gl_draw_elements(
                nr_vertices
            );
        }
        else {
            device.gl_draw_elements_instanced(
                nr_vertices,
                nr_instances
            );
        }
    }
    else {
        if(nr_instances == 1) {
            device.gl_draw_arrays(
                nr_vertices
            );
        }
        else {
            device.gl_draw_arrays_instanced(
                nr_vertices,
                nr_instances
            );
        }
    }

    // end execution context
    device.gl_end();
}

}