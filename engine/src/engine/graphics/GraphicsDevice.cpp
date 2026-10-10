#include <engine/graphics/GraphicsDevice.h>
#include <engine/utils/error.h>

#include <fstream>

namespace engine::graphics {

std::unique_ptr<Shader> GraphicsDevice::create_shader(const fs::path& source_path) {
    // load source code
    std::string source_code;
    {
        std::ifstream file(source_path, std::ios::in | std::ios::binary);
        if(!file) {
            engine::utils::throw_error("Cannot open file : " + source_path.string());
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        source_code = ss.str();
    }

    // infer shader stage via extension
    ShaderStage shader_stage;
    if(source_path.extension() == ".vert") {
        shader_stage = ShaderStage::Vertex;
    } 
    else if(source_path.extension() == ".frag") {
        shader_stage = ShaderStage::Fragment;
    } 
    else if(source_path.extension() == ".compute") {
        shader_stage = ShaderStage::Compute;
    }
    else {
        engine::utils::throw_error("Unknown shader extension: " + source_path.extension().string());
    }

    // create shader
    return create_shader(
        static_cast<std::string_view>(source_code),
        shader_stage
    );
}

}