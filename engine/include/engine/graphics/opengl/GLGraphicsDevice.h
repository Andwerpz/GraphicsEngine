#pragma once 
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include <glad/glad.h>

#include "../GraphicsDevice.h"
#include "GLBuffer.h"
#include "GLTexture.h"
#include "GLSampler.h"
#include "GLVertexArray.h"
#include "GLRenderbuffer.h"
#include "GLFramebuffer.h"
#include "GLRenderTarget.h"
#include "GLShader.h"
#include "GLProgram.h"
#include "GLWindow.h"
#include "GLContext.h"

class GLFWwindow;

namespace engine::graphics::opengl {

// meant to be a thin wrapper over underlying OpenGL graphics API 
// should also track opengl state to provide some cheap sanity checking

class GLGraphicsDevice : public engine::graphics::GraphicsDevice {
    friend class GLBuffer;
    friend class GLTexture;
    friend class GLSampler;
    friend class GLVertexArray;
    friend class GLRenderbuffer;
    friend class GLFramebuffer;
    friend class GLTextureRenderTarget;
    friend class GLWindowRenderTarget;
    friend class GLShader;
    friend class GLGraphicsProgram;
    friend class GLComputeProgram;
    friend class GLContext;
    
public:
    // -- INIT --
    GLGraphicsDevice();
    
    // -- UTILS --
    static GLenum gl_internal_format(TextureFormat format);
    static GLenum gl_format(TextureFormat format);
    static GLenum gl_type(TextureFormat format);
    static GLenum gl_shader_type(ShaderStage stage);
    static GLenum gl_draw_mode(PrimitiveTopology primitive_topology);
    static GLenum gl_index_type(IndexFormat format);
    static uint64_t gl_index_type_size(IndexFormat format);
    static uint64_t gl_vertices_per_primitive(PrimitiveTopology primitive_topology);

    // drains all errors and prints them out
    void gl_get_error();

    // -- CONTEXT MANAGEMENT --
    // sets context to the headless context
    void gl_reset_context();
    void gl_set_context(GLContext& context);

    // gets the currently bound context
    GLContext& gl_get_context();

    // deletes the given context
    // if the given context is currently the one bound, binds the headless context first
    void gl_delete_context(GLContext& context);

    // -- RESOURCES --
    // these are for creating, managing, and deleting resources
    // they can be ran whenever, inside or outside of an execution context

    // resource creation
    GLBuffer gl_create_buffer(
        uint64_t size, 
        BufferUsage usage,
        BufferFrequency frequency,
        const void* data
    );
    GLTexture gl_create_texture(
        TextureDimension type, 
        TextureFormat format, 
        uint32_t width, 
        uint32_t height = 1, 
        uint32_t depth = 1, 
        uint32_t mip_levels = 1
    );
    GLSampler gl_create_sampler(
        SamplerFilter min_filter,
        SamplerFilter mag_filter,
        SamplerMipmapFilter mipmap_filter,
        SamplerAddressMode address_u,
        SamplerAddressMode address_v,
        SamplerAddressMode address_w
    );
    GLVertexArray gl_create_vertex_array();
    GLRenderbuffer gl_create_renderbuffer(
        TextureFormat format,
        uint32_t width,
        uint32_t height
    );
    GLFramebuffer gl_create_framebuffer();
    GLFramebuffer gl_create_window_framebuffer(
        GLWindow& window
    );
    GLShader gl_create_shader(
        ShaderStage stage, 
        std::string source,
        std::vector<uint32_t> spirv_code
    );
    GLRenderingProgram gl_create_rendering_program(
        GLShader& vertex_shader, 
        GLShader& fragment_shader, 
        PrimitiveTopology primitive_topology
    );
    GLComputeProgram gl_create_compute_program(GLShader& compute_shader);

    // resource deletion
    void gl_delete_buffer(GLuint handle);
    void gl_delete_texture(GLuint handle);
    void gl_delete_sampler(GLuint handle);
    void gl_delete_vertex_array(GLuint handle);
    void gl_delete_renderbuffer(GLuint handle);
    void gl_delete_framebuffer(GLuint handle);
    void gl_delete_shader(GLuint handle);
    void gl_delete_rendering_program(GLuint handle);
    void gl_delete_compute_program(GLuint handle);

    // -- EXECUTION --
    // resource binding
    void gl_bind_buffer(uint32_t slot, GLBuffer& buf);
    void gl_bind_sampler(uint32_t slot, GLSampler& sampler);
    void gl_bind_texture(uint32_t slot, GLTexture& tex);
    void gl_bind_vertex_array(GLVertexArray& va);
    void gl_bind_framebuffer(GLFramebuffer& fb);
    void gl_bind_rendering_program(GLRenderingProgram& program);
    void gl_bind_compute_program(GLComputeProgram& program);

    // misc state
    void gl_set_viewport(
        float min_x,
        float min_y,
        float width,
        float height,
        float min_z = 0.0,
        float max_z = 1.0
    );
    void gl_set_scissor(
        int32_t min_x,
        int32_t min_y,
        uint32_t width,
        uint32_t height
    );

    // execution
    void gl_draw_arrays(
        uint32_t primitive_count,
        uint32_t first_primitive = 0
    );
    void gl_draw_elements(
        uint32_t primitive_count,
        uint32_t first_primitive = 0
    );
    void gl_draw_arrays_instanced(
        uint32_t primitive_count, 
        uint32_t instance_count,
        uint32_t first_primitive = 0,
        uint32_t first_instance = 0
    );
    void gl_draw_elements_instanced(
        uint32_t primitive_count, 
        uint32_t instance_count,
        uint32_t first_primitive = 0,
        uint32_t first_instance = 0
    );
    void gl_dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1);
    // void gl_barrier(...);

    // -- EXTERNAL API --
    std::unique_ptr<engine::graphics::Buffer> create_buffer(
        uint64_t size, 
        BufferUsage usage,
        BufferFrequency frequency,
        const void* data
    ) override;
    std::unique_ptr<engine::graphics::Texture> create_texture(
        TextureDimension type, 
        TextureFormat format, 
        uint32_t width, 
        uint32_t height, 
        uint32_t depth, 
        uint32_t mip_levels
    ) override;
    std::unique_ptr<engine::graphics::Sampler> create_sampler(
        SamplerFilter min_filter,
        SamplerFilter mag_filter,
        SamplerMipmapFilter mipmap_filter,
        SamplerAddressMode address_u,
        SamplerAddressMode address_v,
        SamplerAddressMode address_w
    ) override;

    std::unique_ptr<engine::graphics::TextureRenderTarget> create_render_target(
        uint32_t width,
        uint32_t height
    ) override;
    std::unique_ptr<engine::graphics::WindowRenderTarget> create_window_render_target(
        Window& window
    ) override;

    std::unique_ptr<engine::graphics::Shader> create_shader(
        const std::string_view& source,
        ShaderStage stage,
        const fs::path& source_path
    ) override;

    std::unique_ptr<engine::graphics::RenderingProgram> create_rendering_program(
        engine::graphics::Shader& vertex, 
        engine::graphics::Shader& fragment,
        PrimitiveTopology primitive_topology
    ) override;
    std::unique_ptr<engine::graphics::ComputeProgram> create_compute_program(
        engine::graphics::Shader& compute
    ) override;

    std::unique_ptr<Window> create_window(
        uint32_t width,
        uint32_t height,
        const char* title
    ) override;

    void present(Window& window) override;
    uint32_t get_vertex_format_location_count(VertexFormat format) override;

private:
    // hidden window created to initialize OpenGL graphics device
    // this should be used to create all other windows so they can share resources
    // populated during init
    // this context cannot be deleted while this device is still alive
    std::optional<GLContext> headless_context;

    // represents the OpenGL context that is currently actively bound
    // some resources (VertexArrays, Framebuffers, etc.) are context specific 
    // is initialized to headless_context
    GLContext* bound_context = nullptr;

    // stores per-context OpenGL state
    struct GLContextState {
        GLint MAX_UNIFORM_BUFFER_BINDINGS;
        GLint MAX_STORAGE_BUFFER_BINDINGS;
        GLint MAX_TEXTURE_BINDINGS;

        std::vector<std::optional<GLBuffer*>> bound_uniform_buffers;
        std::vector<std::optional<GLBuffer*>> bound_storage_buffers;
        std::vector<std::optional<GLTexture*>> bound_textures;
        std::vector<std::optional<GLSampler*>> bound_samplers;

        std::optional<GLVertexArray*> bound_vertex_array;
        std::optional<GLFramebuffer*> bound_framebuffer;

        std::optional<GLRenderingProgram*> bound_rendering_program;
        std::optional<GLComputeProgram*> bound_compute_program;

        // TODO store viewport and scissor state as well
    };
    std::map<GLFWwindow*, GLContextState> context_state;

    // creates a GLContext and does whatever other bookeeping required 
    GLContext gl_create_context(GLFWwindow* glfw_window);

    // gets the context state for whatever the currently bound context is
    GLContextState& gl_get_context_state();
};

}

