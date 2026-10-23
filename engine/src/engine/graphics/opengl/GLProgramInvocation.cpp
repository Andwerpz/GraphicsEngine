#include <engine/graphics/opengl/GLProgramInvocation.h>
#include <engine/graphics/opengl/GLGraphicsDevice.h>
#include <engine/util/error.h>

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
    // TODO maybe move these checks into RenderingProgramInvocation
    // make sure render target has been bound
    if(!render_target.has_value()) {
        engine::util::throw_error("no render target has been bound");
    }

    // check shader interface inputs
    for(const auto& [location, input_var] : program.get_interface().inputs) {
        // make sure there is a corresponding vertex attribute bound
        if(!attribute_bindings.contains(location)) {
            engine::util::throw_error("no vertex attribute bound for input variable : " + input_var.name);
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
        //     engine::util::throw_error("input variable \"" + input_var.name + "\" has format " + to_string(input_var.type) + " while vertex attribute has format : " + to_string(expected_type));
        // }
    }

    // get render target context
    GLContext* context = nullptr;
    assert(render_target.has_value());
    assert(render_target.value().get().is_validated());
    if(dynamic_cast<GLTextureRenderTarget*>(&(render_target.value().get())) != nullptr) {
        // render in headless context
        GLTextureRenderTarget& gl_target = dynamic_cast<GLTextureRenderTarget&>(render_target.value().get());
        context = &gl_target.get_framebuffer().get_context();
    }
    else if(dynamic_cast<GLWindowRenderTarget*>(&(render_target.value().get())) != nullptr) {
        // render in window context
        GLWindowRenderTarget& gl_target = dynamic_cast<GLWindowRenderTarget&>(render_target.value().get());
        context = &gl_target.get_framebuffer().get_context();
    }
    else assert(false);
    assert(context != nullptr);

    // create vertex array 
    gl_vertex_array.emplace(gl_device.gl_create_vertex_array(*context));
    for(const auto& [location, binding] : attribute_bindings) {
        assert(dynamic_cast<GLBuffer*>(&binding.buffer) != nullptr);
        GLBuffer& gl_buffer = dynamic_cast<GLBuffer&>(binding.buffer);
        VertexFormat format = binding.format;
        gl_vertex_array.value().gl_bind_vertex_buffer(
            location,
            gl_buffer,
            binding.offset,
            binding.stride,
            binding.rate
        );

        switch(format) {
            case VertexFormat::Float32x1:
            case VertexFormat::Float32x2:
            case VertexFormat::Float32x3:
            case VertexFormat::Float32x4:
                gl_vertex_array.value().gl_bind_vertex_attribute(
                    location,
                    location,
                    binding.format,
                    0
                );
                break;
            case VertexFormat::Float32x4x4:
                for(int i = 0; i < 4; i++) {
                    gl_vertex_array.value().gl_bind_vertex_attribute(
                        location + i,
                        location,
                        VertexFormat::Float32x4,
                        16 * i
                    );
                }
                break;
            default:
                assert(false);
        }
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
    uint32_t nr_primitives, 
    uint32_t nr_instances,
    uint32_t first_primitive,
    uint32_t first_instance
) {
    // bind render target
    GLContext* context;
    if(dynamic_cast<GLTextureRenderTarget*>(&(render_target.value().get())) != nullptr) {
        // render in headless context
        GLTextureRenderTarget& gl_target = dynamic_cast<GLTextureRenderTarget&>(render_target.value().get());
        gl_device.gl_bind_framebuffer(gl_target.get_framebuffer());
        context = &gl_target.get_framebuffer().get_context();
    }
    else if(dynamic_cast<GLWindowRenderTarget*>(&(render_target.value().get())) != nullptr) {
        // render in window context
        GLWindowRenderTarget& gl_target = dynamic_cast<GLWindowRenderTarget&>(render_target.value().get());
        gl_device.gl_bind_framebuffer(gl_target.get_framebuffer());
        context = &gl_target.get_framebuffer().get_context();
    }
    else assert(false);
    assert(context != nullptr);

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
            engine::util::throw_error("set must be 0");
        }

        switch(resource.type) {
            case ShaderResourceType::UniformBuffer: {
                assert(resource.uniform_buffer.buffer != nullptr);
                assert(dynamic_cast<GLBuffer*>(resource.uniform_buffer.buffer));
                GLBuffer& gl_buffer = dynamic_cast<GLBuffer&>(*resource.uniform_buffer.buffer);
                gl_device.gl_bind_buffer(*context, binding, gl_buffer);
                break;
            }
            case ShaderResourceType::SampledTexture: {
                assert(resource.sampled_texture.texture != nullptr);
                assert(resource.sampled_texture.sampler != nullptr);
                assert(dynamic_cast<GLTexture*>(resource.sampled_texture.texture));
                assert(dynamic_cast<GLSampler*>(resource.sampled_texture.sampler));
                GLTexture& gl_texture = dynamic_cast<GLTexture&>(*resource.sampled_texture.texture);
                GLSampler& gl_sampler = dynamic_cast<GLSampler&>(*resource.sampled_texture.sampler);
                gl_device.gl_bind_texture(*context, binding, gl_texture);
                gl_device.gl_bind_sampler(*context, binding, gl_sampler);
                break;
            }
            default:
                assert(false);
        }
    }

    // GL_UPPER_LEFT / GL_LOWER_LEFT chooses whether +y or -y is treated as row 0
    // GL_UPPER_LEFT maps +y to row 0, GL_LOWER_LEFT maps -y to row 0
    // when we're rendering to a texture, we can keep the convention that the top row is the first row
    //   this matches image uploads and the high level API top left origin convention
    // when we're rendering to a window, the default framebuffer will always place row 0 at the bottom of 
    //   the screen, so we're forced to map -y to row 0 
    if(dynamic_cast<GLTextureRenderTarget*>(&(render_target.value().get())) != nullptr) {
        // rendering to texture
        glClipControl(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
    }
    else if(dynamic_cast<GLWindowRenderTarget*>(&(render_target.value().get())) != nullptr) {
        // rendering to display
        glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
    }
    else assert(false);

    // set viewport
    gl_device.gl_set_viewport(
        *context, 
        0, 0, 
        render_target.value().get().get_width(),
        render_target.value().get().get_height()
    );

    // clear render target
    // do this before setting clip rect as internally this disables scissor to clear the entire render target
    glClearColor(0.0, 0.0, 0.0, 0.0);
    render_target.value().get().clear(clear_flags);

    // set clip
    if(clipping) {
        glEnable(GL_SCISSOR_TEST);
        gl_device.gl_set_scissor(
            *context,
            clip_min_x, clip_min_y,
            clip_width, clip_height
        );
    }
    else {
        glDisable(GL_SCISSOR_TEST);
    }

    // misc settings 
    // TODO move these raw OpenGL calls into GLGraphicsDevice where the context is managed
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    if(depth_testing) 
        glEnable(GL_DEPTH_TEST);
    else 
        glDisable(GL_DEPTH_TEST);
    
    // execute program
    gl_device.gl_bind_rendering_program(*context, gl_program);
    if(gl_vertex_array.value().has_index_buffer()) {
        if(nr_instances == 1 && first_instance == 0) {
            gl_device.gl_draw_elements(
                *context, 
                nr_primitives,
                first_primitive
            );
        }
        else {
            gl_device.gl_draw_elements_instanced(
                *context, 
                nr_primitives,
                nr_instances,
                first_primitive,
                first_instance
            );
        }
    }
    else {
        if(nr_instances == 1 && first_instance == 0) {
            gl_device.gl_draw_arrays(
                *context, 
                nr_primitives,
                first_primitive
            );
        }
        else {
            gl_device.gl_draw_arrays_instanced(
                *context, 
                nr_primitives,
                nr_instances,
                first_primitive,
                first_instance
            );
        }
    }
}

}