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
    gl_program(program) 
{
    // do nothing
}

void GLRenderingProgramInvocation::draw_impl(
    uint32_t nr_primitives, 
    uint32_t nr_instances,
    uint32_t first_primitive,
    uint32_t first_instance
) {
    // bind render target
    GLContext* context = nullptr;
    assert(render_target.has_value());
    if(dynamic_cast<GLTextureRenderTarget*>(&render_target.value()->get_root()) != nullptr) {
        // render in headless context
        GLTextureRenderTarget& gl_target = dynamic_cast<GLTextureRenderTarget&>(render_target.value()->get_root());
        gl_device.gl_bind_framebuffer(gl_target.get_framebuffer());
        context = &gl_target.get_framebuffer().get_context();
    }
    else if(dynamic_cast<GLWindowRenderTarget*>(&render_target.value()->get_root()) != nullptr) {
        // render in window context
        GLWindowRenderTarget& gl_target = dynamic_cast<GLWindowRenderTarget&>(render_target.value()->get_root());
        gl_device.gl_bind_framebuffer(gl_target.get_framebuffer());
        context = &gl_target.get_framebuffer().get_context();
    }
    else assert(false);
    assert(context != nullptr);

    // handle dirty flags
    {
        auto bind_vertex_attribute = [](
            GLVertexArray& gl_vertex_array, 
            uint32_t location, 
            const VertexAttribute& attribute
        ) {
            assert(dynamic_cast<GLBuffer*>(attribute.buffer) != nullptr);
            GLBuffer& gl_buffer = dynamic_cast<GLBuffer&>(*attribute.buffer);
            VertexFormat format = attribute.format;

            // sanity check that the OpenGL format we're using takes up the same amount of locations
            // as the shader interface vertex attribute
            uint32_t format_locations = format == VertexFormat::Float32x4x4 ? 4 : 1;
            if(format_locations != attribute.nr_locations) {
                engine::util::throw_error(
                    "format spans " + std::to_string(format_locations) +
                    " locations but input variable uses " + std::to_string(attribute.nr_locations)
                );
            }

            // bind buffer to vertex array
            gl_vertex_array.gl_bind_vertex_buffer(
                location,
                gl_buffer,
                attribute.offset,
                attribute.stride,
                attribute.rate
            );

            // bind attributes to buffer
            switch(format) {
                case VertexFormat::Float32x1:
                case VertexFormat::Float32x2:
                case VertexFormat::Float32x3:
                case VertexFormat::Float32x4:
                    gl_vertex_array.gl_bind_vertex_attribute(
                        location,
                        location,
                        attribute.format,
                        0
                    );
                    break;
                case VertexFormat::Float32x4x4:
                    for(int i = 0; i < 4; i++) {
                        gl_vertex_array.gl_bind_vertex_attribute(
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
        };
        auto bind_index_buffer = [](
            GLVertexArray& gl_vertex_array, 
            const IndexBufferBinding& index_buffer
        ) {
            assert(dynamic_cast<GLBuffer*>(index_buffer.buffer) != nullptr);
            GLBuffer& gl_buffer = dynamic_cast<GLBuffer&>(*index_buffer.buffer);
            gl_vertex_array.gl_bind_index_buffer(
                gl_buffer,
                index_buffer.format
            );
        };

        // handle dirty render target
        if(dirty_render_target) {
            // need to re-make vertex array
            gl_vertex_array = std::make_unique<GLVertexArray>(std::move(gl_device.gl_create_vertex_array(*context)));

            // bind everything to this vertex array
            for(const auto& [location, attribute] : attributes) {
                bind_vertex_attribute(*gl_vertex_array, location, attribute);
            }
            if(index_buffer.has_value()) {
                bind_index_buffer(*gl_vertex_array, index_buffer.value());
            }
        }
        else {
            // just need to re-bind stuff that changed to existing vertex array
            assert(gl_vertex_array.get() != nullptr);
            for(uint32_t location : dirty_attributes) {
                assert(attributes.contains(location));
                bind_vertex_attribute(*gl_vertex_array, location, attributes.at(location));
            }
            if(dirty_index_buffer) {
                if(index_buffer.has_value()) {
                    // bind new index buffer
                    bind_index_buffer(*gl_vertex_array, index_buffer.value());
                }
                else {
                    // unbind existing index buffer
                    // TODO
                    engine::util::throw_error("need to support unbinding index buffers");
                }
            }
        }
        dirty_render_target = false;
        dirty_index_buffer = false;
        dirty_attributes.clear();
    }

    // bind vertex array
    assert(gl_vertex_array.get() != nullptr);
    gl_device.gl_bind_vertex_array(*gl_vertex_array);

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
    RenderTarget* root_target = &render_target.value()->get_root();
    if(dynamic_cast<GLTextureRenderTarget*>(root_target) != nullptr) {
        // rendering to texture
        glClipControl(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
    }
    else if(dynamic_cast<GLWindowRenderTarget*>(root_target) != nullptr) {
        // rendering to display
        glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
    }
    else assert(false);

    // set viewport
    int32_t view_xoff = render_target.value()->get_xoff();
    int32_t view_yoff = render_target.value()->get_yoff();
    uint32_t view_width = render_target.value()->get_width();
    uint32_t view_height = render_target.value()->get_height();
    gl_device.gl_set_viewport(
        *context, 
        view_xoff, view_yoff,
        view_width, view_height
    );

    // clear render target
    // do this before setting clip rect as internally this disables scissor to clear the entire render target
    glClearColor(0.0, 0.0, 0.0, 0.0);
    render_target.value()->clear(clear_flags);

    // set clip
    if(clipping) {
        glEnable(GL_SCISSOR_TEST);
        gl_device.gl_set_scissor(
            *context,
            view_xoff + clip_min_x, view_yoff + clip_min_y,
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
    bool do_depth_testing = depth_testing.has_value() ? depth_testing.value() : render_target.value()->has_depth_buffer();
    if(do_depth_testing)    glEnable(GL_DEPTH_TEST);
    else                    glDisable(GL_DEPTH_TEST);
    
    // execute program
    gl_device.gl_bind_rendering_program(*context, gl_program);
    if(gl_vertex_array->has_index_buffer()) {
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