#pragma once
#include <cstdint>
#include <memory>
#include <filesystem>
namespace fs = std::filesystem;

#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"
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
    // -- EXTERNAL API --
    // GPU resources
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

    // should compile the given shader for the underlying graphics device 
    virtual std::unique_ptr<Shader> create_shader(
        fs::path source_path
    ) = 0;

    // create various types of programs using combinations of shaders
    virtual std::unique_ptr<RenderingProgram> create_rendering_program(
        Shader& vertex, 
        Shader& fragment,
        PrimitiveTopology primitive_topology
    ) = 0;
    virtual std::unique_ptr<ComputeProgram> create_compute_program(
        Shader& compute
    ) = 0;

    // make whatever was rendered since the last present() visible
    virtual void present() = 0;

    // window
    virtual Window& get_window() = 0;
};

}

