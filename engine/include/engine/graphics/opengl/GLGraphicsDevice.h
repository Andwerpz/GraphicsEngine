#pragma once 
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include <glad/glad.h>

#include "engine/graphics/GraphicsDevice.h"
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

namespace engine::graphics::opengl {

// meant to be a thin wrapper over underlying OpenGL graphics API 
// mostly here for convenience and so that users don't have to directly use OpenGL

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
    
public:
    // -- INIT --
    static std::unique_ptr<engine::graphics::GraphicsDevice> create(uint32_t width, uint32_t height, const char* title);
    GLGraphicsDevice(GLWindow&& window);
    ~GLGraphicsDevice() = default;
    
    // -- UTILS --
    static GLenum gl_internal_format(TextureFormat format);
    static GLenum gl_format(TextureFormat format);
    static GLenum gl_type(TextureFormat format);
    static GLenum gl_shader_type(ShaderStage stage);
    static GLenum gl_draw_mode(PrimitiveTopology primitive_topology);
    static GLenum gl_index_type(IndexFormat format);

    // drains all errors and prints them out
    void gl_get_error();

    // -- WINDOW --
    GLWindow& gl_get_window() {
        return window;
    }

    GLFramebuffer gl_get_window_framebuffer() {
        // rendering to framebuffer 0 goes to the window
        return GLFramebuffer(*this, 0);
    }

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
    GLShader gl_create_shader(
        ShaderStage stage, 
        std::string source_code,
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
    // all methods here need to be ran between a begin() and end()
    // these will modify the execution state until the next end() 
    // when you begin(), you should have no inherited execution state 

    // state management
    void gl_begin();
    void gl_end();

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
    void gl_draw_arrays(uint32_t vertex_count);
    void gl_draw_elements(uint32_t vertex_count);
    void gl_draw_arrays_instanced(uint32_t vertex_count, uint32_t instance_count);
    void gl_draw_elements_instanced(uint32_t vertex_count, uint32_t instance_count);
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
    std::unique_ptr<engine::graphics::WindowRenderTarget> create_window_render_target() override;

    std::unique_ptr<engine::graphics::Shader> create_shader(
        fs::path source_path
    ) override;

    std::unique_ptr<engine::graphics::RenderingProgram> create_rendering_program(
        engine::graphics::Shader& vertex, 
        engine::graphics::Shader& fragment,
        PrimitiveTopology primitive_topology
    ) override;
    std::unique_ptr<engine::graphics::ComputeProgram> create_compute_program(
        engine::graphics::Shader& compute
    ) override;

    void present() override;
    engine::graphics::Window& get_window() override;

private:
    // the window that this graphics device is connected to
    GLWindow window;

    // whether or not we are inside an execution context
    bool executing = false;

    std::vector<std::optional<GLBuffer*>> bound_uniform_buffers;
    std::vector<std::optional<GLBuffer*>> bound_storage_buffers;
    std::vector<std::optional<GLTexture*>> bound_textures;
    std::vector<std::optional<GLSampler*>> bound_samplers;

    std::optional<GLVertexArray*> bound_vertex_array;
    std::optional<GLFramebuffer*> bound_framebuffer;

    bool program_bound = false;
    std::optional<GLRenderingProgram*> bound_rendering_program;
    std::optional<GLComputeProgram*> bound_compute_program;
};

}

