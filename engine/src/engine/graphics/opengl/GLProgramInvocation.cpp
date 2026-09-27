#include <engine/graphics/opengl/GLProgramInvocation.h>
#include <engine/graphics/opengl/GLGraphicsDevice.h>
#include <engine/utils/error.h>

#include <iostream>

namespace engine::graphics::opengl {

GLRenderingProgramInvocation::GLRenderingProgramInvocation(
    GLGraphicsDevice& device,
    GLRenderingProgram& program
) : RenderingProgramInvocation(device, program), 
    gl_device(device),
    gl_program(program) {
    gl_vertex_array = std::nullopt;
}

void GLRenderingProgramInvocation::validate_impl() {
    // make sure that all inputs are bound

    // make sure no two outputs are bound to the same texture
    
    // make sure no two textures are bound to the same output

    // create vertex array 
    gl_vertex_array.emplace(gl_device.gl_create_vertex_array());
    for(VertexAttributeBinding& binding : attribute_bindings) {
        GLBuffer& gl_buffer = dynamic_cast<GLBuffer&>(binding.buffer);
        gl_vertex_array.value().gl_bind_vertex_buffer(
            binding.location,
            gl_buffer,
            binding.offset,
            binding.stride
        );
    }
    if(index_buffer.has_value()) {
        GLBuffer& gl_buffer = dynamic_cast<GLBuffer&>(index_buffer.value().buffer);
        gl_vertex_array.value().gl_bind_index_buffer(
            gl_buffer,
            index_buffer.value().format
        );
    }

    // create framebuffer
    if(bound_window) {
        gl_framebuffer.emplace(gl_device.gl_get_window_framebuffer());
    }
    else {
        gl_framebuffer.emplace(gl_device.gl_create_framebuffer());
        std::vector<GLenum> output_mapping(program.get_output_location_extent(), GL_NONE);
        uint32_t color_attachment_ind = 0;
        for(OutputBinding& binding : output_bindings) {
            GLTexture& gl_texture = dynamic_cast<GLTexture&>(binding.texture);
            gl_framebuffer.value().gl_bind_color_attachment(
                color_attachment_ind,
                gl_texture
            );
            output_mapping[binding.location] = GL_COLOR_ATTACHMENT0 + color_attachment_ind;
        }
        gl_framebuffer.value().gl_set_draw_buffers(output_mapping);
        if(!gl_framebuffer.value().gl_is_complete()) {
            throw std::runtime_error("validate_impl failed to create framebuffer");
        }
    }
}

void GLRenderingProgramInvocation::draw_impl(
    uint32_t nr_vertices, 
    uint32_t nr_instances
) {
    // start execution context
    gl_device.gl_begin();

    // bind resources
    for(auto i = resources.begin(); i != resources.end(); i++) {
        auto location = i->first;
        uint32_t set = location.first;
        uint32_t binding = location.second;
        Resource& resource = i->second;

        // opengl doesn't have the notion of descriptor sets 
        if(set != 0) {
            engine::utils::throw_error("set must be 0");
        }

        if(dynamic_cast<Buffer*>(&resource)) {
            assert(dynamic_cast<GLBuffer*>(&resource));
            GLBuffer& gl_buffer = dynamic_cast<GLBuffer&>(resource);
            gl_device.gl_bind_buffer(binding, gl_buffer);
        }
        else assert(false);
    }

    // bind vertex array
    assert(gl_vertex_array.has_value());
    gl_device.gl_bind_vertex_array(gl_vertex_array.value());

    // bind framebuffer
    assert(gl_framebuffer.has_value());
    gl_device.gl_bind_framebuffer(gl_framebuffer.value());

    // execute program
    gl_device.gl_bind_rendering_program(gl_program);
    if(gl_vertex_array.value().has_index_buffer()) {
        if(nr_instances == 1) {
            gl_device.gl_draw_elements(
                nr_vertices
            );
        }
        else {
            gl_device.gl_draw_elements_instanced(
                nr_vertices,
                nr_instances
            );
        }
    }
    else {
        if(nr_instances == 1) {
            gl_device.gl_draw_arrays(
                nr_vertices
            );
        }
        else {
            gl_device.gl_draw_arrays_instanced(
                nr_vertices,
                nr_instances
            );
        }
    }

    // end execution context
    gl_device.gl_end();
}

}