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
    // TODO

    // make sure no two outputs are bound to the same texture
    // TODO
    
    // make sure no two textures are bound to the same output
    // TODO

    // make sure all bound textures have the same dimension
    // TODO

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
        gl_vertex_array.value().gl_bind_vertex_attribute(
            binding.location,
            binding.location,
            binding.format,
            0
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
        
        // bind color attachments
        std::vector<GLenum> output_mapping(program.get_output_location_extent(), GL_NONE);
        uint32_t color_attachment_ind = 0;
        for(OutputBinding& binding : output_bindings) {
            GLTexture& gl_texture = dynamic_cast<GLTexture&>(binding.texture);
            gl_framebuffer.value().gl_bind_color_attachment(
                color_attachment_ind,
                gl_texture
            );
            output_mapping[binding.location] = GL_COLOR_ATTACHMENT0 + color_attachment_ind;
            color_attachment_ind ++;
        }
        gl_framebuffer.value().gl_set_draw_buffers(output_mapping);

        // bind depth buffer
        if(bound_depth_buffer) {
            // TODO right now it's not too clear to me how i'm going to derive the dimensions of the depth buffer
            //  maybe all RenderingProgramInvocations should have a clear dimension
            //  or maybe the idea of a 'dimensioned framebuffer' should be made part of the high level API
            //  and a RenderingProgramInvocation just renders to this 'RenderTarget' abstraction
            assert(false);
            // gl_depth_buffer.emplace(gl_device.gl_create_renderbuffer(
            //     TextureFormat::Depth32F,
            //     width,
            //     height
            // ));
            // gl_framebuffer.value().gl_bind_depth_attachment(gl_depth_buffer.value());
        }

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
    for(auto i = bound_resources.begin(); i != bound_resources.end(); i++) {
        auto location = i->first;
        uint32_t set = location.first;
        uint32_t binding = location.second;
        ResourceBinding& resource = i->second;

        // opengl doesn't have the notion of descriptor sets 
        if(set != 0) {
            engine::utils::throw_error("set must be 0");
        }

        switch(resource.type) {
            case ShaderResourceType::UniformBuffer: {
                assert(resource.uniform_buffer.buffer != nullptr);
                assert(dynamic_cast<GLBuffer*>(resource.uniform_buffer.buffer));
                GLBuffer& gl_buffer = dynamic_cast<GLBuffer&>(*resource.uniform_buffer.buffer);
                gl_device.gl_bind_buffer(binding, gl_buffer);
                break;
            }
            case ShaderResourceType::SampledTexture: {
                assert(resource.sampled_texture.texture != nullptr);
                assert(resource.sampled_texture.sampler != nullptr);
                assert(dynamic_cast<GLTexture*>(resource.sampled_texture.texture));
                assert(dynamic_cast<GLSampler*>(resource.sampled_texture.sampler));
                GLTexture& gl_texture = dynamic_cast<GLTexture&>(*resource.sampled_texture.texture);
                GLSampler& gl_sampler = dynamic_cast<GLSampler&>(*resource.sampled_texture.sampler);
                gl_device.gl_bind_texture(binding, gl_texture);
                gl_device.gl_bind_sampler(binding, gl_sampler);
                break;
            }
            default:
                assert(false);
        }
    }

    // bind vertex array
    assert(gl_vertex_array.has_value());
    gl_device.gl_bind_vertex_array(gl_vertex_array.value());

    // bind framebuffer
    assert(gl_framebuffer.has_value());
    gl_device.gl_bind_framebuffer(gl_framebuffer.value());

    // execute program
    if(depth_testing) {
        glEnable(GL_DEPTH_TEST);
    }
    if(clear_color || clear_depth) {
        GLenum clear_flags = 
            (clear_color ? GL_COLOR_BUFFER_BIT : 0) |
            (clear_depth ? GL_DEPTH_BUFFER_BIT : 0);
        glClear(clear_flags);
    }
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