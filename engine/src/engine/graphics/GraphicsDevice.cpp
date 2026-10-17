#include <engine/graphics/GraphicsDevice.h>
#include <engine/util/error.h>
#include <engine/util/file.h>

#include <fstream>

namespace engine::graphics {

std::unique_ptr<Shader> GraphicsDevice::create_shader(const fs::path& source_path) {
    // load source code
    std::string source_code = engine::util::load_text_file(source_path);

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
        engine::util::throw_error("Unknown shader extension: " + source_path.extension().string());
    }

    // create shader
    return create_shader(
        static_cast<std::string_view>(source_code),
        shader_stage
    );
}

}