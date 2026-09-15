#pragma once 
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include <glad/glad.h>

#include "engine/graphics/GraphicsDevice.h"
#include "GLBuffer.h"
#include "GLTexture.h"
#include "GLProgram.h"
#include "GLFramebuffer.h"
#include "GLShader.h"
#include "Window.h"

namespace engine::graphics::opengl {

// meant to be a thin wrapper over underlying OpenGL graphics API 
// mostly here for convenience and so that users don't have to directly use OpenGL

class OpenGLGraphicsDevice : public engine::graphics::GraphicsDevice {
    friend class GLBuffer;
    friend class GLTexture;
    friend class GLFramebuffer;
    friend class GLShader;
    friend class GLGraphicsProgram;
    friend class GLComputeProgram;
    
public:
    // -- INIT --
    static std::unique_ptr<engine::graphics::GraphicsDevice> create(uint32_t width, uint32_t height, const char* title);
    OpenGLGraphicsDevice(Window&& window);
    ~OpenGLGraphicsDevice() = default;

    // -- WINDOW --
    Window& gl_get_window() {
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
        GLBufferMemory memory
    );
    GLTexture gl_create_texture(
        GLTextureType type, 
        GLTextureFormat format, 
        uint32_t width, 
        uint32_t height = 1, 
        uint32_t depth = 1, 
        uint32_t mip_levels = 1
    );
    GLFramebuffer gl_create_framebuffer();
    GLShader gl_create_shader(GLShaderType type, const std::string& source_code);
    GLGraphicsProgram gl_create_graphics_pipeline(
        GLShader& vertex_shader, 
        GLShader& fragment_shader, 
        PrimitiveTopology primitive_topology
    );
    GLComputeProgram gl_create_compute_pipeline(GLShader& compute_shader);

    // resource management
    void gl_upload_buffer(
        GLBuffer& dst,
        const void* data,
        uint64_t size,
        uint64_t offset = 0
    );
    void gl_copy_buffer(
        GLBuffer& src, 
        GLBuffer& dst,
        uint64_t size,
        uint64_t src_offset = 0,
        uint64_t dst_offset = 0
    );

    void gl_upload_texture(
        GLTexture& texture,
        const void* data,
        uint32_t mip_level = 0,
        uint32_t cube_face = 0
    );
    void gl_generate_mipmaps(GLTexture& texture);

    // resource deletion
    void gl_delete_buffer(uint64_t handle);
    void gl_delete_texture(uint64_t handle);
    void gl_delete_framebuffer(uint64_t handle);
    void gl_delete_shader(uint64_t handle);
    void gl_delete_graphics_pipeline(uint64_t handle);
    void gl_delete_compute_pipeline(uint64_t handle);

    // -- EXECUTION --
    // all methods here need to be ran between a begin() and end()
    // these will modify the execution state until the next end() 
    // when you begin(), you should have no inherited execution state 

    // state management
    void gl_begin();
    void gl_end();

    // resource binding
    void gl_bind_texture(uint32_t slot, GLTexture& tex);
    void gl_bind_buffer(uint32_t slot, GLBufferUsage usage, GLBuffer& buf);
    void gl_bind_framebuffer(GLFramebuffer& tgt);
    void gl_bind_graphics_pipeline(GLGraphicsProgram& pipeline);
    void gl_bind_compute_pipeline(GLComputeProgram& pipeline);

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
    void gl_draw(uint32_t vertex_count, uint32_t first_vertex = 0);
    void gl_dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1);
    // void gl_barrier(...);

    // -- PIPELINE --
    std::unique_ptr<engine::pipeline::Pipeline> create_pipeline() override;
    std::unique_ptr<engine::pipeline::Program> create_program(std::vector<engine::pipeline::Shader*> shaders) override;
    std::unique_ptr<engine::pipeline::Shader> create_shader(fs::path source_path) override;

private:
    // the window that this graphics device is connected to
    Window window;

    // whether or not we are inside an execution context
    bool executing = false;

    uint32_t max_bound_textures;
    std::vector<GLint> bound_textures;

    uint32_t max_bound_buffers;
    std::vector<std::pair<GLint, GLBufferUsage>> bound_buffers;

    GLint bound_framebuffer;
    GLint bound_program;
};

}

