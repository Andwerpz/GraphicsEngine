#include <engine/pipeline/Shader.h>
#include <engine/graphics/opengl/OpenGLGraphicsDevice.h>

#include <filesystem>
namespace fs = std::filesystem;

#include <iostream>
#include <chrono>
#include <thread>

int main(int argc, char** argv) {
    auto device = engine::graphics::opengl::OpenGLGraphicsDevice::create(800, 600, "test window");
    // std::this_thread::sleep_for(std::chrono::seconds(10));

    fs::path shader_path = fs::current_path() / "res" / "shaders" / "testing" / "test.frag" ;
    std::cout << "SHADER PATH : " << shader_path << std::endl;
    if(!fs::exists(shader_path)) {
        std::cout << "shader doesn't exist" << std::endl;
        return 1;
    }

    auto shader = device->create_shader(shader_path);
}