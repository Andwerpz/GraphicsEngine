#include <engine/graphics/opengl/GLGraphicsDevice.h>
#include <engine/graphics/GraphicsDevice.h>
#include <engine/graphics/Shader.h>
#include <glm/glm.hpp>

#include <engine/graphics/opengl/GLBuffer.h>

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
    
    fs::path vertex_path = fs::current_path() / "res" / "shaders" / "testing" / "uniform.vert" ;
    fs::path fragment_path = fs::current_path() / "res" / "shaders" / "testing" / "uniform.frag" ;

    auto program = create_rendering_program(*device, vertex_path, fragment_path);

    auto invocation = program->create_invocation(); 
    invocation->bind_window();
    invocation->validate();

    // auto token = invocation->get_uniform<float>("bar.foo2.arr[1][67]");
    // auto token = invocation->get_uniform<float>("bar.asdf");

    auto token = invocation->get_uniform<glm::vec3>("ucolor.col");
    std::cout << "Offset : " << token.offset << std::endl;

    invocation->set_uniform(token, {1, 1, 1});
    
    if(false) {
        engine::graphics::opengl::GLBuffer& gl_buffer = dynamic_cast<engine::graphics::opengl::GLBuffer&>(invocation->get_resource({0, 2}));
        if(!glIsBuffer(gl_buffer.get_handle())) {
            std::cerr << "Not a valid OpenGL buffer: " << gl_buffer.get_handle() << '\n';
        }

        auto check_gl_error = [](const char* operation) {
            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                std::cerr
                    << operation
                    << " failed: 0x"
                    << std::hex << error
                    << std::dec << '\n';
            }
        };

        float upload_data[4] = {
            1.0f,
            0.5f,
            0.25f,
            0.0f
        };
        glNamedBufferSubData(
            gl_buffer.get_handle(),
            0,
            sizeof(upload_data),
            upload_data
        );
        check_gl_error("glNamedBufferSubData");

        float read_data[4] = {};
        glGetNamedBufferSubData(
            gl_buffer.get_handle(),
            0,
            sizeof(read_data),
            read_data
        );
        check_gl_error("glGetNamedBufferSubData");
        std::cout
            << read_data[0] << " "
            << read_data[1] << " "
            << read_data[2] << " "
            << read_data[3] << '\n';
    }

    invocation->draw(3);
    device->present();

    std::this_thread::sleep_for(std::chrono::seconds(5));

}