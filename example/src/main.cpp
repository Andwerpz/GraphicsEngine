#include <engine/graphics/opengl/GLGraphicsDevice.h>
#include <engine/graphics/GraphicsDevice.h>
#include <engine/graphics/Shader.h>

#include <filesystem>
namespace fs = std::filesystem;

#include <iostream>
#include <chrono>
#include <thread>

std::unique_ptr<engine::graphics::Shader> create_shader(engine::graphics::GraphicsDevice& device, fs::path path) {
    if(!fs::exists(path)) {
        throw std::runtime_error("shader doesn't exist : " + path.string());
    }
    return device.create_shader(path);
}

std::unique_ptr<engine::graphics::RenderingProgram> create_rendering_program(engine::graphics::GraphicsDevice& device, fs::path vertex_path, fs::path fragment_path) {
    auto vertex_shader = create_shader(device, vertex_path);
    auto fragment_shader = create_shader(device, fragment_path);
    return device.create_rendering_program(
        *vertex_shader, 
        *fragment_shader, 
        engine::graphics::PrimitiveTopology::TriangleList
    );
}

int main(int argc, char** argv) {
    auto device = engine::graphics::opengl::GLGraphicsDevice::create(800, 600, "test window");
    
    fs::path vertex_path = fs::current_path() / "res" / "shaders" / "testing" / "test.vert" ;
    fs::path fragment_path = fs::current_path() / "res" / "shaders" / "testing" / "test.frag" ;

    auto program = create_rendering_program(*device, vertex_path, fragment_path);

    auto invocation = program->create_invocation(); 
    invocation->bind_window();
    invocation->validate();
    invocation->draw(3);

    device->present();

    std::this_thread::sleep_for(std::chrono::seconds(5));

}