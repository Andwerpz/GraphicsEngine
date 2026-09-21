#pragma once
#include <cstdint>
#include <memory>
#include <filesystem>
namespace fs = std::filesystem;

#include "Buffer.h"
#include "Texture.h"
#include "Shader.h"
#include "Program.h"
#include "ProgramInvocation.h"

namespace engine::graphics {

class RenderingProgram;
class ComputeProgram;

class GraphicsDevice {
protected:
    using BufferMemory = engine::graphics::BufferMemory;
    using TextureType = engine::graphics::TextureType;
    using TextureFormat = engine::graphics::TextureFormat;
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
        BufferMemory memory
    ) = 0;
    virtual std::unique_ptr<Texture> create_texture(
        TextureType type, 
        TextureFormat format, 
        uint32_t width, 
        uint32_t height, 
        uint32_t depth, 
        uint32_t mip_levels
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
};

}

