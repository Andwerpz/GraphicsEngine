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
    // check shader interface inputs
    for(const auto& [location, input_var] : program.get_interface().inputs) {
        // make sure there is a corresponding vertex attribute bound
        if(!attribute_bindings.contains(location)) {
            engine::utils::throw_error("no vertex attribute bound for input variable : " + input_var.name);
        }

        // make sure vertex attribute format matches type of input variable
        // TODO expand this to handle conversions 
        //   for example, a byte array could be interpreted as an array of vec4s

        // VertexFormat format = attribute_bindings.at(location).format;
        // ShaderType expected_type = ShaderType(ShaderVoidType());
        // switch(format) {
        //     case VertexFormat::Float32x1:
        //         expected_type = ShaderType(ShaderScalarType::Float);
        //         break;
        //     case VertexFormat::Float32x2:
        //         expected_type = ShaderType(ShaderVectorType(ShaderScalarType::Float, 2));
        //         break;
        //     case VertexFormat::Float32x3:
        //         expected_type = ShaderType(ShaderVectorType(ShaderScalarType::Float, 3));
        //         break;
        //     case VertexFormat::Float32x4:
        //         expected_type = ShaderType(ShaderVectorType(ShaderScalarType::Float, 4));
        //         break;
        //     default:
        //         assert(false);
        // }
        // if(!(input_var.type == expected_type)) {
        //     engine::utils::throw_error("input variable \"" + input_var.name + "\" has format " + to_string(input_var.type) + " while vertex attribute has format : " + to_string(expected_type));
        // }
    }

    // bind render target context
    assert(render_target.has_value());
    assert(render_target.value().get().is_validated());
    if(dynamic_cast<GLTextureRenderTarget*>(&(render_target.value().get())) != nullptr) {
        // render in headless context
        GLTextureRenderTarget& gl_target = dynamic_cast<GLTextureRenderTarget&>(render_target.value().get());
        gl_device.gl_reset_context();
    }
    else if(dynamic_cast<GLWindowRenderTarget*>(&(render_target.value().get())) != nullptr) {
        // render in window context
        GLWindowRenderTarget& gl_target = dynamic_cast<GLWindowRenderTarget&>(render_target.value().get());
        gl_device.gl_set_context(gl_target.get_window().get_context());
    }
    else assert(false);

    // create vertex array 
    gl_vertex_array.emplace(gl_device.gl_create_vertex_array());
    for(const auto& [location, binding] : attribute_bindings) {
        assert(dynamic_cast<GLBuffer*>(&binding.buffer) != nullptr);
        GLBuffer& gl_buffer = dynamic_cast<GLBuffer&>(binding.buffer);
        gl_vertex_array.value().gl_bind_vertex_buffer(
            location,
            gl_buffer,
            binding.offset,
            binding.stride
        );
        gl_vertex_array.value().gl_bind_vertex_attribute(
            location,
            location,
            binding.format,
            0
        );
    }
    if(index_buffer.has_value()) {
        assert(dynamic_cast<GLBuffer*>(&index_buffer.value().buffer) != nullptr);
        GLBuffer& gl_buffer = dynamic_cast<GLBuffer&>(index_buffer.value().buffer);
        gl_vertex_array.value().gl_bind_index_buffer(
            gl_buffer,
            index_buffer.value().format
        );
    }
}

void GLRenderingProgramInvocation::draw_impl(
    uint32_t nr_vertices, 
    uint32_t nr_instances
) {
    // start execution context
    gl_device.gl_begin();

    // bind render target
    if(dynamic_cast<GLTextureRenderTarget*>(&(render_target.value().get())) != nullptr) {
        // render in headless context
        GLTextureRenderTarget& gl_target = dynamic_cast<GLTextureRenderTarget&>(render_target.value().get());
        gl_device.gl_reset_context();
        gl_device.gl_bind_framebuffer(gl_target.get_framebuffer());
    }
    else if(dynamic_cast<GLWindowRenderTarget*>(&(render_target.value().get())) != nullptr) {
        // render in window context
        GLWindowRenderTarget& gl_target = dynamic_cast<GLWindowRenderTarget&>(render_target.value().get());
        gl_device.gl_set_context(gl_target.get_window().get_context());
        gl_device.gl_bind_framebuffer(gl_target.get_framebuffer());
    }
    else assert(false);

    // bind vertex array
    assert(gl_vertex_array.has_value());
    gl_device.gl_bind_vertex_array(gl_vertex_array.value());

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

    // misc settings
    if(depth_testing) {
        glEnable(GL_DEPTH_TEST);
    }
    if(clear_color || clear_depth) {
        GLenum clear_flags = 
            (clear_color ? GL_COLOR_BUFFER_BIT : 0) |
            (clear_depth ? GL_DEPTH_BUFFER_BIT : 0);
        glClear(clear_flags);
    }
    gl_device.gl_set_viewport(
        0, 0, 
        render_target.value().get().get_width(),
        render_target.value().get().get_height()
    );

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