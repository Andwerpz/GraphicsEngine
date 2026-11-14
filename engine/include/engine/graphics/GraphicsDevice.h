#pragma once
#include <cstdint>
#include <memory>
#include <filesystem>
namespace fs = std::filesystem;

#include <engine/util/macro.h>

#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"
#include "RenderTarget.h"
#include "Shader.h"
#include "Program.h"
#include "ProgramInvocation.h"
#include "Window.h"

namespace engine::graphics {

class RenderingProgram;
class ComputeProgram;

class GraphicsDevice {
protected:
    using BufferUsage = engine::graphics::BufferUsage;
    using BufferFrequency = engine::graphics::BufferFrequency;
    using TextureDimension = engine::graphics::TextureDimension;
    using TextureFormat = engine::graphics::TextureFormat;
    using SamplerFilter = engine::graphics::SamplerFilter;
    using SamplerMipmapFilter = engine::graphics::SamplerMipmapFilter;
    using SamplerAddressMode = engine::graphics::SamplerAddressMode;
    using PrimitiveTopology = engine::graphics::PrimitiveTopology;
    using ShaderStage = engine::graphics::ShaderStage;
    using VertexInputRate = engine::graphics::VertexInputRate;
    using VertexFormat = engine::graphics::VertexFormat;
    using IndexFormat = engine::graphics::IndexFormat;

public:
    GraphicsDevice() = default;

    // make GraphicsDevice non-moveable to let it have a stable address once allocated
    NON_COPYABLE_NON_MOVABLE(GraphicsDevice);

    // -- RESOURCES --
    virtual std::unique_ptr<Buffer> create_buffer(
        uint64_t size,
        BufferUsage usage,
        BufferFrequency frequency,
        const void* data = nullptr
    ) = 0;
    virtual std::unique_ptr<Texture> create_texture(
        TextureDimension type, 
        TextureFormat format, 
        uint32_t width, 
        uint32_t height, 
        uint32_t depth, 
        uint32_t mip_levels
    ) = 0;
    virtual std::unique_ptr<Sampler> create_sampler(
        SamplerFilter min_filter = SamplerFilter::Linear,
        SamplerFilter mag_filter = SamplerFilter::Linear,
        SamplerMipmapFilter mipmap_filter = SamplerMipmapFilter::None,
        SamplerAddressMode address_u = SamplerAddressMode::Repeat,
        SamplerAddressMode address_v = SamplerAddressMode::Repeat,
        SamplerAddressMode address_w = SamplerAddressMode::Repeat
    ) = 0;

    // should return an unvalidated texture render target
    virtual std::unique_ptr<TextureRenderTarget> create_render_target(
        uint32_t width,
        uint32_t height
    ) = 0;

    // should return a pre-validated window render target
    // the provided window should be originally created by this graphics device
    virtual std::unique_ptr<WindowRenderTarget> create_window_render_target(
        Window& window
    ) = 0;

    // should compile the given shader for the underlying graphics device 
    virtual std::unique_ptr<Shader> create_shader(
        const std::string_view& source,
        ShaderStage stage,
        const fs::path& source_path = "path_not_provided"
    ) = 0;
    std::unique_ptr<Shader> create_shader(const fs::path& source_path);

    // create various types of programs using combinations of shaders
    virtual std::unique_ptr<RenderingProgram> create_rendering_program(
        Shader& vertex, 
        Shader& fragment,
        PrimitiveTopology primitive_topology
    ) = 0;
    virtual std::unique_ptr<ComputeProgram> create_compute_program(
        Shader& compute
    ) = 0;
    
    // TODO
    // // create various types of program invocations using combinations of shaders
    // // it just wraps the factory method to create the corresponding program
    // //   and makes the invocation own the program
    // std::unique_ptr<RenderingProgramInvocation> create_rendering_program_invocation(
    //     Shader& vertex, 
    //     Shader& fragment,
    //     PrimitiveTopology primitive_topology
    // );

    // creates a new window managed by this graphics device
    virtual std::unique_ptr<Window> create_window(
        uint32_t width = 800,
        uint32_t height = 600,
        const char* title = "my window"
    ) = 0;

    // -- ACTIONS --
    // make whatever was rendered since the last present() visible
    virtual void present(Window& window) = 0;
};

}

