#pragma once 
#include <cstdint>
#include <string>
#include <vector>

#include <glad/glad.h>

#include "Window.h"
#include "Buffer.h"
#include "Texture.h"
#include "Program.h"
#include "RenderTarget.h"
#include "Shader.h"

namespace engine::graphics::opengl {

// meant to be a thin wrapper over underlying OpenGL graphics API 
// mostly here for convenience and so that users don't have to directly use OpenGL

class GraphicsDevice {
    friend class Buffer;
    friend class Texture;
    friend class RenderTarget;
    friend class Shader;
    friend class GraphicsProgram;
    friend class ComputeProgram;
    
public:
    // -- INIT --
    static GraphicsDevice* create(uint32_t width, uint32_t height, const char* title);
    GraphicsDevice(Window&& window);
    ~GraphicsDevice() = default;

    // -- WINDOW --
    Window& get_window() {
        return window;
    }

    RenderTarget& get_window_render_target() {
        return window_render_target;
    }

    // -- RESOURCES --
    // these are for creating, managing, and deleting resources
    // they can be ran whenever, inside or outside of an execution context

    // resource creation
    Buffer create_buffer(
        uint64_t size, 
        BufferMemory memory
    );
    Texture create_texture(
        TextureType type, 
        TextureFormat format, 
        uint32_t width, 
        uint32_t height = 1, 
        uint32_t depth = 1, 
        uint32_t mip_levels = 1
    );
    RenderTarget create_render_target();
    Shader create_shader(ShaderType type, const std::string& source_code);
    GraphicsProgram create_graphics_pipeline(
        Shader& vertex_shader, 
        Shader& fragment_shader, 
        PrimitiveTopology primitive_topology
    );
    ComputeProgram create_compute_pipeline(Shader& compute_shader);

    // resource management
    void upload_buffer(
        Buffer& dst,
        const void* data,
        uint64_t size,
        uint64_t offset = 0
    );
    void copy_buffer(
        Buffer& src, 
        Buffer& dst,
        uint64_t size,
        uint64_t src_offset = 0,
        uint64_t dst_offset = 0
    );

    void upload_texture(
        Texture& texture,
        const void* data,
        uint32_t mip_level = 0,
        uint32_t cube_face = 0
    );
    void generate_mipmaps(Texture& texture);

    // resource deletion
    void delete_buffer(uint64_t handle);
    void delete_texture(uint64_t handle);
    void delete_render_target(uint64_t handle);
    void delete_shader(uint64_t handle);
    void delete_graphics_pipeline(uint64_t handle);
    void delete_compute_pipeline(uint64_t handle);

    // -- EXECUTION --
    // all methods here need to be ran between a begin() and end()
    // these will modify the execution state until the next end() 
    // when you begin(), you should have no inherited execution state 

    // state management
    void begin();
    void end();

    // resource binding
    void bind_texture(uint32_t slot, Texture& tex);
    void bind_buffer(uint32_t slot, BufferUsage usage, Buffer& buf);
    void bind_render_target(RenderTarget& tgt);
    void bind_graphics_pipeline(GraphicsProgram& pipeline);
    void bind_compute_pipeline(ComputeProgram& pipeline);

    // misc state
    void set_viewport(
        float min_x,
        float min_y,
        float width,
        float height,
        float min_z = 0.0,
        float max_z = 1.0
    );
    void set_scissor(
        int32_t min_x,
        int32_t min_y,
        uint32_t width,
        uint32_t height
    );

    // execution
    void draw(uint32_t vertex_count, uint32_t first_vertex = 0);
    void dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1);
    // void barrier(...);

private:
    // the window that this graphics device is connected to
    Window window;

    // rendering to this target should render to the underlying window
    RenderTarget window_render_target;

    // whether or not we are inside an execution context
    bool executing = false;

    uint32_t max_bound_textures;
    std::vector<GLint> bound_textures;

    uint32_t max_bound_buffers;
    std::vector<std::pair<GLint, BufferUsage>> bound_buffers;

    GLint bound_framebuffer;
    GLint bound_program;
};

}

