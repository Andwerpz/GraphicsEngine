#include <engine/graphics/opengl/GLGraphicsDevice.h>
#include <engine/graphics/GraphicsDevice.h>
#include <engine/graphics/Shader.h>
#include <engine/graphics/Buffer.h>
#include <engine/assets/Image.h>

#include <engine/graphics/opengl/GLBuffer.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

std::unique_ptr<engine::graphics::RenderingProgram> create_rendering_program(
    engine::graphics::GraphicsDevice& device, 
    fs::path vertex_path, 
    fs::path fragment_path,
    engine::graphics::PrimitiveTopology primitive_topology
) {
    auto vertex_shader = create_shader(device, vertex_path);
    auto fragment_shader = create_shader(device, fragment_path);
    return device.create_rendering_program(
        *vertex_shader, 
        *fragment_shader, 
        primitive_topology
    );
}

constexpr float cube_vertices[] = {
    // Front (+Z)
    -0.5f, -0.5f,  0.5f,
    0.5f, -0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

    // Back (-Z)
    0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,

    0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,

    // Left (-X)
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,

    // Right (+X)
    0.5f, -0.5f,  0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,

    0.5f, -0.5f,  0.5f,
    0.5f,  0.5f, -0.5f,
    0.5f,  0.5f,  0.5f,

    // Top (+Y)
    -0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    0.5f,  0.5f, -0.5f,

    -0.5f,  0.5f,  0.5f,
    0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,

    // Bottom (-Y)
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
};

constexpr float cube_normals[] = {
    // Front (+Z)
     0,  0,  1,
     0,  0,  1,
     0,  0,  1,
     0,  0,  1,
     0,  0,  1,
     0,  0,  1,

    // Back (-Z)
     0,  0, -1,
     0,  0, -1,
     0,  0, -1,
     0,  0, -1,
     0,  0, -1,
     0,  0, -1,

    // Left (-X)
    -1,  0,  0,
    -1,  0,  0,
    -1,  0,  0,
    -1,  0,  0,
    -1,  0,  0,
    -1,  0,  0,

    // Right (+X)
     1,  0,  0,
     1,  0,  0,
     1,  0,  0,
     1,  0,  0,
     1,  0,  0,
     1,  0,  0,

    // Top (+Y)
     0,  1,  0,
     0,  1,  0,
     0,  1,  0,
     0,  1,  0,
     0,  1,  0,
     0,  1,  0,

    // Bottom (-Y)
     0, -1,  0,
     0, -1,  0,
     0, -1,  0,
     0, -1,  0,
     0, -1,  0,
     0, -1,  0,
};

constexpr float cube_uvs[] = {
    // Front (+Z)
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    // Back (-Z)
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    // Left (-X)
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    // Right (+X)
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    // Top (+Y)
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    // Bottom (-Y)
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
};

// make sure that setting uniforms works
// should render a screen that oscillates between black and white
void test_uniform() {
    auto device = engine::graphics::opengl::GLGraphicsDevice::create(800, 600, "test window");
    fs::path vertex_path = fs::current_path() / "res/shaders/testing/uniform.vert";
    fs::path fragment_path = fs::current_path() / "res/shaders/testing/uniform.frag";
    auto program = create_rendering_program(
        *device, 
        vertex_path, 
        fragment_path,
        engine::graphics::PrimitiveTopology::TriangleList
    );

    auto window_render_target = device->create_window_render_target();

    auto invocation = program->create_invocation(); 
    invocation->bind_render_target(*window_render_target);
    invocation->validate();

    // auto token = invocation->get_uniform<float>("bar.foo2.arr[1][67]");
    // auto token = invocation->get_uniform<float>("bar.asdf");

    auto token = invocation->get_uniform<glm::vec3>("ucolor.col");
    std::cout << "Offset : " << token.offset << std::endl;
    
    auto start_time = std::chrono::steady_clock::now();
    while(true) {
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - start_time).count();

        // 1 Hz sine wave, mapped from [-1, 1] to [0, 1].
        float value = static_cast<float>((std::sin(elapsed * 2.0 * M_PI) + 1.0) * 0.5);
        invocation->set_uniform(token, glm::vec3(value));
        invocation->draw(3);
        device->present();

        std::this_thread::sleep_for(std::chrono::nanoseconds(1000000000 / 60));
    }
}

// make sure that rendering arrays of triangles works
// should render a rotating, shaded cube
void test_cube() {
    auto device = engine::graphics::opengl::GLGraphicsDevice::create(800, 600, "test window");
    fs::path vertex_path = fs::current_path() / "res/shaders/testing/cube.vert";
    fs::path fragment_path = fs::current_path() / "res/shaders/testing/cube.frag";
    auto program = create_rendering_program(
        *device, 
        vertex_path, 
        fragment_path,
        engine::graphics::PrimitiveTopology::TriangleList
    );

    auto cube_vertex_buffer = device->create_buffer(
        sizeof(cube_vertices),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_vertices
    );
    auto cube_normal_buffer = device->create_buffer(
        sizeof(cube_normals),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_normals
    );
    
    auto window_render_target = device->create_window_render_target();
    auto invocation = program->create_invocation(); 
    invocation->bind_render_target(*window_render_target);
    invocation->bind_vertex_attribute(
        0,      // location
        *cube_vertex_buffer,
        0,      // offset
        12,     // stride
        engine::graphics::VertexInputRate::Vertex,
        engine::graphics::VertexFormat::Float32x3
    );
    invocation->bind_vertex_attribute(
        1,      // location
        *cube_normal_buffer,
        0,      // offset
        12,     // stride
        engine::graphics::VertexInputRate::Vertex,
        engine::graphics::VertexFormat::Float32x3
    );
    invocation->validate();

    auto& window = device->get_window();
    uint32_t window_width = window.get_width();
    uint32_t window_height = window.get_height();

    auto projection_token = invocation->get_uniform<glm::mat4>("camera.projection");
    auto view_token = invocation->get_uniform<glm::mat4>("camera.view");

    while(true) {
        float time = std::chrono::duration<float>(std::chrono::steady_clock::now().time_since_epoch()).count();
        float angle = time * 1.0f; // radians/sec
        float radius = 2.0f;
        float height = 1.0f;
        glm::vec3 camera_position(
            radius * std::cos(angle),
            height,
            radius * std::sin(angle)
        );

        glm::mat4 projection = glm::perspective(
            glm::radians(90.0f),
            window_width / static_cast<float>(window_height),
            0.1f,
            100.0f
        );
        glm::mat4 view = glm::lookAt(
            camera_position,
            glm::vec3(0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        invocation->set_uniform(projection_token, projection);
        invocation->set_uniform(view_token, view);

        invocation->draw(36);
        device->present();

        dynamic_cast<engine::graphics::opengl::GLGraphicsDevice&>(*device).gl_get_error();

        std::this_thread::sleep_for(std::chrono::nanoseconds(1000000000 / 60));
    }
}

// make sure that texture binding and sampling works
// should render a full screen image
void test_texture() {
    auto device = engine::graphics::opengl::GLGraphicsDevice::create(800, 600, "test window");
    fs::path vertex_path = fs::current_path() / "res/shaders/testing/texture.vert";
    fs::path fragment_path = fs::current_path() / "res/shaders/testing/texture.frag";
    auto program = create_rendering_program(
        *device, 
        vertex_path, 
        fragment_path,
        engine::graphics::PrimitiveTopology::TriangleList
    );

    auto cube_vertex_buffer = device->create_buffer(
        sizeof(cube_vertices),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_vertices
    );
    auto cube_normal_buffer = device->create_buffer(
        sizeof(cube_normals),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_normals
    );
    auto cube_uv_buffer = device->create_buffer(
        sizeof(cube_uvs),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_uvs
    ); 

    auto sampler = device->create_sampler();

    fs::path image_path = fs::current_path() / "res/textures/astolfo.png";
    auto image = engine::assets::Image::load_image(image_path);
    auto texture = device->create_texture(
        engine::graphics::TextureDimension::D2,
        engine::graphics::TextureFormat::RGBA32F,
        image.get_width(),
        image.get_height(),
        1,  // depth
        1   // mip levels
    );
    texture->upload_image(image);

    auto window_render_target = device->create_window_render_target();
    auto invocation = program->create_invocation(); 
    invocation->bind_render_target(*window_render_target);

    invocation->bind_vertex_attribute(
        0,      // location
        *cube_vertex_buffer,
        0,      // offset
        12,     // stride
        engine::graphics::VertexInputRate::Vertex,
        engine::graphics::VertexFormat::Float32x3
    );
    invocation->bind_vertex_attribute(
        1,      // location
        *cube_normal_buffer,
        0,      // offset
        12,     // stride
        engine::graphics::VertexInputRate::Vertex,
        engine::graphics::VertexFormat::Float32x3
    );
    invocation->bind_vertex_attribute(
        2,      // location
        *cube_uv_buffer,
        0,      // offset
        8,      // stride
        engine::graphics::VertexInputRate::Vertex,
        engine::graphics::VertexFormat::Float32x2
    );

    invocation->bind_sampled_texture(
        {0, 1},
        *texture,
        *sampler
    );

    invocation->validate();

    auto& window = device->get_window();
    uint32_t window_width = window.get_width();
    uint32_t window_height = window.get_height();

    auto projection_token = invocation->get_uniform<glm::mat4>("camera.projection");
    auto view_token = invocation->get_uniform<glm::mat4>("camera.view");

    while(true) {
        float time = std::chrono::duration<float>(std::chrono::steady_clock::now().time_since_epoch()).count();
        float angle = time * 1.0f; // radians/sec
        float radius = 2.0f;
        float height = 1.0f;
        glm::vec3 camera_position(
            radius * std::cos(angle),
            height,
            radius * std::sin(angle)
        );

        glm::mat4 projection = glm::perspective(
            glm::radians(90.0f),
            window_width / static_cast<float>(window_height),
            0.1f,
            100.0f
        );
        glm::mat4 view = glm::lookAt(
            camera_position,
            glm::vec3(0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        invocation->set_uniform(projection_token, projection);
        invocation->set_uniform(view_token, view);

        invocation->draw(36);
        device->present();

        dynamic_cast<engine::graphics::opengl::GLGraphicsDevice&>(*device).gl_get_error();

        std::this_thread::sleep_for(std::chrono::nanoseconds(1000000000 / 60));
    }
}

// make sure cubemaps work
void test_skybox() {
    auto device = engine::graphics::opengl::GLGraphicsDevice::create(800, 600, "test window");
    auto window_render_target = device->create_window_render_target();

    auto cube_program = create_rendering_program(
        *device,
        fs::current_path() / "res/shaders/testing/texture.vert", 
        fs::current_path() / "res/shaders/testing/texture.frag",
        engine::graphics::PrimitiveTopology::TriangleList
    );
    auto skybox_program = create_rendering_program(
        *device, 
        fs::current_path() / "res/shaders/testing/skybox.vert", 
        fs::current_path() / "res/shaders/testing/skybox.frag",
        engine::graphics::PrimitiveTopology::TriangleList
    );

    auto cube_vertex_buffer = device->create_buffer(
        sizeof(cube_vertices),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_vertices
    );
    auto cube_normal_buffer = device->create_buffer(
        sizeof(cube_normals),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_normals
    );
    auto cube_uv_buffer = device->create_buffer(
        sizeof(cube_uvs),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_uvs
    ); 

    auto sampler = device->create_sampler();

    auto texture_image = engine::assets::Image::load_image(fs::current_path() / "res/textures/astolfo.png");
    auto texture = device->create_texture(
        engine::graphics::TextureDimension::D2,
        engine::graphics::TextureFormat::RGBA32F,
        texture_image.get_width(),
        texture_image.get_height(),
        1,  // depth
        1   // mip levels
    );
    texture->upload_image(texture_image);

    auto** skybox_image = (engine::assets::Image**) malloc(sizeof(engine::assets::Image*) * 6);
    const char* skybox_strs[] = {
        "right.jpg",   // +X
        "left.jpg",    // -X
        "top.jpg",     // +Y
        "bottom.jpg",  // -Y
        "front.jpg",   // +Z
        "back.jpg",    // -Z
    };
    for(int i = 0; i < 6; i++) {
        auto image = engine::assets::Image::load_image(fs::current_path() / "res/textures/lake" / skybox_strs[i]);
        skybox_image[i] = new engine::assets::Image(std::move(image));
    }
    auto skybox = device->create_texture(
        engine::graphics::TextureDimension::Cube,
        engine::graphics::TextureFormat::RGBA32F,
        skybox_image[0]->get_width(),
        skybox_image[0]->get_height(),
        1,
        1
    );
    for(int i = 0; i < 6; i++) {
        skybox->upload_image(*skybox_image[i], 0, i);
    }

    auto skybox_invocation = skybox_program->create_invocation(); 
    {
        skybox_invocation->bind_render_target(*window_render_target);

        skybox_invocation->bind_vertex_attribute(
            0,      // location
            *cube_vertex_buffer,
            0,      // offset
            12,     // stride
            engine::graphics::VertexInputRate::Vertex,
            engine::graphics::VertexFormat::Float32x3
        );
        skybox_invocation->bind_vertex_attribute(
            1,      // location
            *cube_normal_buffer,
            0,      // offset
            12,     // stride
            engine::graphics::VertexInputRate::Vertex,
            engine::graphics::VertexFormat::Float32x3
        );
        skybox_invocation->bind_vertex_attribute(
            2,      // location
            *cube_uv_buffer,
            0,      // offset
            8,      // stride
            engine::graphics::VertexInputRate::Vertex,
            engine::graphics::VertexFormat::Float32x2
        );

        skybox_invocation->bind_sampled_texture(
            {0, 1},
            *skybox,
            *sampler
        );

        skybox_invocation->validate();
    }

    auto cube_invocation = cube_program->create_invocation();
    {
        cube_invocation->bind_render_target(*window_render_target);
        
        cube_invocation->bind_vertex_attribute(
            0,      // location
            *cube_vertex_buffer,
            0,      // offset
            12,     // stride
            engine::graphics::VertexInputRate::Vertex,
            engine::graphics::VertexFormat::Float32x3
        );
        cube_invocation->bind_vertex_attribute(
            1,      // location
            *cube_normal_buffer,
            0,      // offset
            12,     // stride
            engine::graphics::VertexInputRate::Vertex,
            engine::graphics::VertexFormat::Float32x3
        );
        cube_invocation->bind_vertex_attribute(
            2,      // location
            *cube_uv_buffer,
            0,      // offset
            8,      // stride
            engine::graphics::VertexInputRate::Vertex,
            engine::graphics::VertexFormat::Float32x2
        );

        cube_invocation->bind_sampled_texture(
            {0, 1},
            *texture,
            *sampler
        );

        cube_invocation->validate();
    }

    auto& window = device->get_window();
    uint32_t window_width = window.get_width();
    uint32_t window_height = window.get_height();

    auto skybox_projection = skybox_invocation->get_uniform<glm::mat4>("camera.projection");
    auto skybox_view = skybox_invocation->get_uniform<glm::mat4>("camera.view");
    auto cube_projection = cube_invocation->get_uniform<glm::mat4>("camera.projection");
    auto cube_view = cube_invocation->get_uniform<glm::mat4>("camera.view");

    cube_invocation->do_clear_color(false);
    cube_invocation->do_clear_depth(true);

    while(true) {
        float time = std::chrono::duration<float>(std::chrono::steady_clock::now().time_since_epoch()).count();
        float angle = time * 1.0f; // radians/sec
        float radius = 2.0f;
        float height = 1.0f;
        glm::vec3 camera_position(
            radius * std::cos(angle),
            height,
            radius * std::sin(angle)
        );

        glm::mat4 projection = glm::perspective(
            glm::radians(90.0f),
            window_width / static_cast<float>(window_height),
            0.1f,
            100.0f
        );
        glm::mat4 view = glm::lookAt(
            camera_position,
            glm::vec3(0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        skybox_invocation->set_uniform(skybox_projection, projection);
        skybox_invocation->set_uniform(skybox_view, view);
        skybox_invocation->draw(36);

        cube_invocation->set_uniform(cube_projection, projection);
        cube_invocation->set_uniform(cube_view, view);
        cube_invocation->draw(36);

        device->present();

        dynamic_cast<engine::graphics::opengl::GLGraphicsDevice&>(*device).gl_get_error();

        std::this_thread::sleep_for(std::chrono::nanoseconds(1000000000 / 60));
    }
}

// make sure we can render to a texture via texture render target
void test_render_target() {
    auto device = engine::graphics::opengl::GLGraphicsDevice::create(800, 600, "test window");

    auto cube_program = create_rendering_program(
        *device, 
        fs::current_path() / "res/shaders/testing/cube.vert", 
        fs::current_path() / "res/shaders/testing/cube.frag",
        engine::graphics::PrimitiveTopology::TriangleList
    );
    auto program = create_rendering_program(
        *device, 
        fs::current_path() / "res/shaders/testing/texture.vert", 
        fs::current_path() / "res/shaders/testing/texture.frag",
        engine::graphics::PrimitiveTopology::TriangleList
    );

    auto cube_vertex_buffer = device->create_buffer(
        sizeof(cube_vertices),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_vertices
    );
    auto cube_normal_buffer = device->create_buffer(
        sizeof(cube_normals),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_normals
    );
    auto cube_uv_buffer = device->create_buffer(
        sizeof(cube_uvs),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_uvs
    );

    auto sampler = device->create_sampler();
    auto cube_texture = device->create_texture(
        engine::graphics::TextureDimension::D2,
        engine::graphics::TextureFormat::RGBA32F,
        512,    // width
        512,    // height
        1,      // depth
        1       // mip levels
    );

    auto window_render_target = device->create_window_render_target();
    auto cube_render_target = device->create_render_target(512, 512);
    cube_render_target->bind_texture(0, *cube_texture);
    cube_render_target->bind_depth_buffer();
    cube_render_target->validate();

    auto cube_invocation = cube_program->create_invocation();
    {
        cube_invocation->bind_render_target(*cube_render_target);
        cube_invocation->bind_vertex_attribute(
            0,      // location
            *cube_vertex_buffer,
            0,      // offset
            12,     // stride
            engine::graphics::VertexInputRate::Vertex,
            engine::graphics::VertexFormat::Float32x3
        );
        cube_invocation->bind_vertex_attribute(
            1,      // location
            *cube_normal_buffer,
            0,      // offset
            12,     // stride
            engine::graphics::VertexInputRate::Vertex,
            engine::graphics::VertexFormat::Float32x3
        );
        cube_invocation->validate();
    }
    
    auto invocation = program->create_invocation(); 
    {
        invocation->bind_render_target(*window_render_target);
        invocation->bind_vertex_attribute(
            0,      // location
            *cube_vertex_buffer,
            0,      // offset
            12,     // stride
            engine::graphics::VertexInputRate::Vertex,
            engine::graphics::VertexFormat::Float32x3
        );
        invocation->bind_vertex_attribute(
            1,      // location
            *cube_normal_buffer,
            0,      // offset
            12,     // stride
            engine::graphics::VertexInputRate::Vertex,
            engine::graphics::VertexFormat::Float32x3
        );
        invocation->bind_vertex_attribute(
            2,      // location
            *cube_uv_buffer,
            0,      // offset
            8,      // stride
            engine::graphics::VertexInputRate::Vertex,
            engine::graphics::VertexFormat::Float32x2
        );
        invocation->bind_sampled_texture(
            {0, 1},
            *cube_texture,
            *sampler
        );

        invocation->validate();
    }

    auto& window = device->get_window();
    uint32_t window_width = window.get_width();
    uint32_t window_height = window.get_height();

    auto cube_projection_token = cube_invocation->get_uniform<glm::mat4>("camera.projection");
    auto cube_view_token = cube_invocation->get_uniform<glm::mat4>("camera.view");

    auto projection_token = invocation->get_uniform<glm::mat4>("camera.projection");
    auto view_token = invocation->get_uniform<glm::mat4>("camera.view");

    while(true) {
        float time = std::chrono::duration<float>(std::chrono::steady_clock::now().time_since_epoch()).count();
        float angle = time * 1.0f; // radians/sec
        float radius = 2.0f;
        float height = 1.0f;
        glm::vec3 camera_position(
            radius * std::cos(angle),
            height,
            radius * std::sin(angle)
        );

        glm::mat4 cube_projection = glm::perspective(
            glm::radians(90.0f),
            512 / static_cast<float>(512),
            0.1f,
            100.0f
        );
        glm::mat4 cube_view = glm::lookAt(
            camera_position,
            glm::vec3(0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        glm::mat4 projection = glm::perspective(
            glm::radians(90.0f),
            window_width / static_cast<float>(window_height),
            0.1f,
            100.0f
        );
        glm::mat4 view = glm::lookAt(
            camera_position,
            glm::vec3(0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        cube_invocation->set_uniform(cube_projection_token, cube_projection);
        cube_invocation->set_uniform(cube_view_token, cube_view);
        cube_invocation->draw(36);

        invocation->set_uniform(projection_token, projection);
        invocation->set_uniform(view_token, view);
        invocation->draw(36);
        device->present();

        dynamic_cast<engine::graphics::opengl::GLGraphicsDevice&>(*device).gl_get_error();

        std::this_thread::sleep_for(std::chrono::nanoseconds(1000000000 / 60));
    }
}

void test_model() {
    
}

int main(int argc, char** argv) {

    // test_uniform();
    // test_cube();
    // test_texture();
    // test_skybox();
    test_render_target();
    // test_model();

    return 0;
}