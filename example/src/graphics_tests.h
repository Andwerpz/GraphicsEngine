#pragma once
#include "utils.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// make sure that setting uniforms works
// should render a screen that oscillates between black and white
inline void test_uniform() {
    engine::graphics::opengl::GLGraphicsDevice gl_device;
    engine::graphics::GraphicsDevice& device = gl_device;
    auto window = device.create_window(800, 600, "test window");
    uint32_t window_width = window->get_width();
    uint32_t window_height = window->get_height();
    auto window_render_target = device.create_window_render_target(*window);

    fs::path vertex_path = fs::current_path() / "res/shaders/testing/uniform.vert";
    fs::path fragment_path = fs::current_path() / "res/shaders/testing/uniform.frag";
    auto program = create_rendering_program(
        device, 
        vertex_path, 
        fragment_path,
        engine::graphics::PrimitiveTopology::TriangleList
    );

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
        invocation->draw(1);
        device.present(*window);

        std::this_thread::sleep_for(std::chrono::nanoseconds(1000000000 / 60));
    }
}

// make sure that rendering arrays of triangles works
// should render a rotating, shaded cube
inline void test_cube() {
    engine::graphics::opengl::GLGraphicsDevice gl_device;
    engine::graphics::GraphicsDevice& device = gl_device;
    auto window = device.create_window(800, 600, "test window");
    uint32_t window_width = window->get_width();
    uint32_t window_height = window->get_height();
    auto window_render_target = device.create_window_render_target(*window);
    
    fs::path vertex_path = fs::current_path() / "res/shaders/testing/cube.vert";
    fs::path fragment_path = fs::current_path() / "res/shaders/testing/cube.frag";
    auto program = create_rendering_program(
        device, 
        vertex_path, 
        fragment_path,
        engine::graphics::PrimitiveTopology::TriangleList
    );

    auto cube_vertex_buffer = device.create_buffer(
        sizeof(cube_vertices),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_vertices
    );
    auto cube_normal_buffer = device.create_buffer(
        sizeof(cube_normals),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_normals
    );
    
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

        invocation->draw(12);
        device.present(*window);

        dynamic_cast<engine::graphics::opengl::GLGraphicsDevice&>(device).gl_get_error();

        std::this_thread::sleep_for(std::chrono::nanoseconds(1000000000 / 60));
    }
}

// make sure that texture binding and sampling works
// should render a full screen image
inline void test_texture() {
    engine::graphics::opengl::GLGraphicsDevice gl_device;
    engine::graphics::GraphicsDevice& device = gl_device;
    auto window = device.create_window(800, 600, "test window");
    uint32_t window_width = window->get_width();
    uint32_t window_height = window->get_height();
    auto window_render_target = device.create_window_render_target(*window);

    fs::path vertex_path = fs::current_path() / "res/shaders/testing/texture.vert";
    fs::path fragment_path = fs::current_path() / "res/shaders/testing/texture.frag";
    auto program = create_rendering_program(
        device, 
        vertex_path, 
        fragment_path,
        engine::graphics::PrimitiveTopology::TriangleList
    );

    auto cube_vertex_buffer = device.create_buffer(
        sizeof(cube_vertices),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_vertices
    );
    auto cube_normal_buffer = device.create_buffer(
        sizeof(cube_normals),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_normals
    );
    auto cube_uv_buffer = device.create_buffer(
        sizeof(cube_uvs),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_uvs
    ); 

    auto sampler = device.create_sampler();

    fs::path image_path = fs::current_path() / "res/textures/astolfo.png";
    auto image = engine::asset::Image::load_image(image_path);
    auto texture = device.create_texture(
        engine::graphics::TextureDimension::D2,
        engine::graphics::TextureFormat::RGBA32F,
        image.get_width(),
        image.get_height(),
        1,  // depth
        1   // mip levels
    );
    texture->upload_image(image);

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

        invocation->draw(12);
        device.present(*window);

        dynamic_cast<engine::graphics::opengl::GLGraphicsDevice&>(device).gl_get_error();

        std::this_thread::sleep_for(std::chrono::nanoseconds(1000000000 / 60));
    }
}

// make sure cubemaps work
inline void test_skybox() {
    engine::graphics::opengl::GLGraphicsDevice gl_device;
    engine::graphics::GraphicsDevice& device = gl_device;
    auto window = device.create_window(800, 600, "test window");
    uint32_t window_width = window->get_width();
    uint32_t window_height = window->get_height();
    auto window_render_target = device.create_window_render_target(*window);

    auto cube_program = create_rendering_program(
        device,
        fs::current_path() / "res/shaders/testing/texture.vert", 
        fs::current_path() / "res/shaders/testing/texture.frag",
        engine::graphics::PrimitiveTopology::TriangleList
    );
    auto skybox_program = create_rendering_program(
        device, 
        fs::current_path() / "res/shaders/testing/skybox.vert", 
        fs::current_path() / "res/shaders/testing/skybox.frag",
        engine::graphics::PrimitiveTopology::TriangleList
    );

    auto cube_vertex_buffer = device.create_buffer(
        sizeof(cube_vertices),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_vertices
    );
    auto cube_normal_buffer = device.create_buffer(
        sizeof(cube_normals),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_normals
    );
    auto cube_uv_buffer = device.create_buffer(
        sizeof(cube_uvs),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_uvs
    ); 

    auto sampler = device.create_sampler();

    auto texture_image = engine::asset::Image::load_image(fs::current_path() / "res/textures/astolfo.png");
    auto texture = device.create_texture(
        engine::graphics::TextureDimension::D2,
        engine::graphics::TextureFormat::RGBA32F,
        texture_image.get_width(),
        texture_image.get_height(),
        1,  // depth
        1   // mip levels
    );
    texture->upload_image(texture_image);

    auto** skybox_image = (engine::asset::Image**) malloc(sizeof(engine::asset::Image*) * 6);
    const char* skybox_strs[] = {
        "right.jpg",   // +X
        "left.jpg",    // -X
        "top.jpg",     // +Y
        "bottom.jpg",  // -Y
        "front.jpg",   // +Z
        "back.jpg",    // -Z
    };
    for(int i = 0; i < 6; i++) {
        auto image = engine::asset::Image::load_image(fs::current_path() / "res/textures/lake" / skybox_strs[i]);
        skybox_image[i] = new engine::asset::Image(std::move(image));
    }
    auto skybox = device.create_texture(
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
        skybox_invocation->draw(12);

        cube_invocation->set_uniform(cube_projection, projection);
        cube_invocation->set_uniform(cube_view, view);
        cube_invocation->draw(12);

        device.present(*window);

        dynamic_cast<engine::graphics::opengl::GLGraphicsDevice&>(device).gl_get_error();

        std::this_thread::sleep_for(std::chrono::nanoseconds(1000000000 / 60));
    }
}

// make sure we can render to a texture via texture render target
inline void test_render_target() {
    engine::graphics::opengl::GLGraphicsDevice gl_device;
    engine::graphics::GraphicsDevice& device = gl_device;
    auto window = device.create_window(800, 600, "test window");
    uint32_t window_width = window->get_width();
    uint32_t window_height = window->get_height();
    auto window_render_target = device.create_window_render_target(*window);

    auto cube_program = create_rendering_program(
        device, 
        fs::current_path() / "res/shaders/testing/cube.vert", 
        fs::current_path() / "res/shaders/testing/cube.frag",
        engine::graphics::PrimitiveTopology::TriangleList
    );
    auto program = create_rendering_program(
        device, 
        fs::current_path() / "res/shaders/testing/texture.vert", 
        fs::current_path() / "res/shaders/testing/texture.frag",
        engine::graphics::PrimitiveTopology::TriangleList
    );

    auto cube_vertex_buffer = device.create_buffer(
        sizeof(cube_vertices),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_vertices
    );
    auto cube_normal_buffer = device.create_buffer(
        sizeof(cube_normals),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_normals
    );
    auto cube_uv_buffer = device.create_buffer(
        sizeof(cube_uvs),
        engine::graphics::BufferUsage::Vertex,
        engine::graphics::BufferFrequency::Static,
        cube_uvs
    );

    auto sampler = device.create_sampler();
    auto cube_texture = device.create_texture(
        engine::graphics::TextureDimension::D2,
        engine::graphics::TextureFormat::RGBA32F,
        512,    // width
        512,    // height
        1,      // depth
        1       // mip levels
    );

    auto cube_render_target = device.create_render_target(512, 512);
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
        cube_invocation->draw(12);

        invocation->set_uniform(projection_token, projection);
        invocation->set_uniform(view_token, view);
        invocation->draw(12);
        device.present(*window);

        dynamic_cast<engine::graphics::opengl::GLGraphicsDevice&>(device).gl_get_error();

        std::this_thread::sleep_for(std::chrono::nanoseconds(1000000000 / 60));
    }
}

inline void test_separate_invocations() {
    using namespace engine::graphics;
    opengl::GLGraphicsDevice gl_device;
    GraphicsDevice& device = gl_device;
    auto window = device.create_window(800, 600, "test window");
    uint32_t window_width = window->get_width();
    uint32_t window_height = window->get_height();
    auto window_render_target = device.create_window_render_target(*window);

    auto program = create_rendering_program(
        device,
        fs::current_path() / "res/shaders/ui/rect.vert",
        fs::current_path() / "res/shaders/ui/rect.frag",
        PrimitiveTopology::TriangleList
    );

    // Each instance consists of:
    //   16 floats: model matrix
    //    4 floats: color
    struct RectInstance {
        glm::mat4 model;
        glm::vec4 color;
    };

    static_assert(sizeof(RectInstance) == 20 * sizeof(float));

    const RectInstance left_rect = {
        glm::translate(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 0.0f))
            * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 1.0f)),
        glm::vec4(1, 0, 0, 1)
    };

    const RectInstance right_rect = {
        glm::translate(glm::mat4(1.0f), glm::vec3(150.0f, 50.0f, 0.0f))
            * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 1.0f)),
        glm::vec4(0, 1, 0, 1)
    };

    auto left_buffer = device.create_buffer(
        sizeof(RectInstance),
        BufferUsage::Vertex,
        BufferFrequency::Dynamic,
        &left_rect
    );

    auto right_buffer = device.create_buffer(
        sizeof(RectInstance),
        BufferUsage::Vertex,
        BufferFrequency::Dynamic,
        &right_rect
    );

    auto left_invocation = program->create_invocation();
    left_invocation->bind_render_target(*window_render_target);

    auto right_invocation = program->create_invocation();
    right_invocation->bind_render_target(*window_render_target);

    auto bind_rect_instance = [](
        auto& invocation,
        auto& buffer
    ) {
        // Matrix occupies attribute locations 0, 1, 2, and 3.
        for (uint32_t column = 0; column < 4; ++column) {
            invocation->bind_vertex_attribute(
                column,
                *buffer,
                column * sizeof(glm::vec4),
                sizeof(RectInstance),
                VertexInputRate::Instance,
                VertexFormat::Float32x4
            );
        }

        invocation->bind_vertex_attribute(
            4,
            *buffer,
            4 * sizeof(glm::vec4),
            sizeof(RectInstance),
            VertexInputRate::Instance,
            VertexFormat::Float32x4
        );

        invocation->validate();
    };

    bind_rect_instance(left_invocation, left_buffer);
    bind_rect_instance(right_invocation, right_buffer);

    glm::mat4 projection = glm::ortho(
        0.0f, static_cast<float>(window->get_width()),
        static_cast<float>(window->get_height()), 0.0f,
        -1.0f, 1.0f
    );
    auto projection_left = left_invocation->get_uniform<glm::mat4>("camera.projection");
    auto projection_right = right_invocation->get_uniform<glm::mat4>("camera.projection");
    left_invocation->set_uniform(projection_left, projection);
    right_invocation->set_uniform(projection_right, projection);

    // Clear only once per frame. The second invocation must preserve
    // the first rectangle.
    right_invocation->do_clear_color(false);
    right_invocation->do_clear_depth(false);

    while (true) {
        left_invocation->draw(2, 1);
        right_invocation->draw(2, 1);
        device.present(*window);

        gl_device.gl_get_error();

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

inline void test_same_invocation_twice() {
    using namespace engine::graphics;
    opengl::GLGraphicsDevice gl_device;
    GraphicsDevice& device = gl_device;
    auto window = device.create_window(800, 600, "test window");
    uint32_t window_width = window->get_width();
    uint32_t window_height = window->get_height();
    auto window_render_target = device.create_window_render_target(*window);

    auto program = create_rendering_program(
        device,
        fs::current_path() / "res/shaders/ui/rect.vert",
        fs::current_path() / "res/shaders/ui/rect.frag",
        PrimitiveTopology::TriangleList
    );

    struct RectInstance {
        glm::mat4 model;
        glm::vec4 color;
    };
    static_assert(sizeof(RectInstance) == 20 * sizeof(float));

    // one buffer, two instance slots: [0] = red, [1] = green
    auto instance_buffer = device.create_buffer(
        2 * sizeof(RectInstance),
        BufferUsage::Vertex,
        BufferFrequency::Dynamic,
        nullptr
    );

    auto invocation = program->create_invocation();
    invocation->bind_render_target(*window_render_target);
    invocation->bind_vertex_attribute(
        0, *instance_buffer, 0, sizeof(RectInstance),
        VertexInputRate::Instance, VertexFormat::Float32x4x4
    );
    invocation->bind_vertex_attribute(
        4, *instance_buffer, 4 * sizeof(glm::vec4), sizeof(RectInstance),
        VertexInputRate::Instance, VertexFormat::Float32x4
    );
    invocation->validate();

    const glm::mat4 projection = glm::ortho(
        0.0f, static_cast<float>(window->get_width()),
        static_cast<float>(window->get_height()), 0.0f,
        -1.0f, 1.0f
    );
    auto projection_token = invocation->get_uniform<glm::mat4>("camera.projection");
    invocation->set_uniform(projection_token, projection);

    auto probe = [&](const char* tag, int x, int y) {
        GLubyte px[4] = {};
        glReadPixels(x, window_height - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
        std::cout << tag << ": " << (int)px[0] << "," << (int)px[1] << "," << (int)px[2] << '\n';
    };

    while (true) {
        std::cout << "FRAME BEGIN : " << std::endl;

        // RectInstance rects[2] = {
        //     { // red
        //         glm::translate(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 0.0f))
        //             * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 1.0f)),
        //         glm::vec4(1, 0, 0, 1)
        //     },
        //     { // green
        //         glm::translate(glm::mat4(1.0f), glm::vec3(150.0f, 50.0f, 0.0f))
        //             * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 1.0f)),
        //         glm::vec4(0, 1, 0, 1)
        //     }
        // };
        // instance_buffer->upload_data(rects, sizeof(rects));

        // red: instance 0
        RectInstance rect = { // red
            glm::translate(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 0.0f))
                * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 1.0f)),
            glm::vec4(1, 0, 0, 1)
        };
        instance_buffer->upload_data(&rect, sizeof(RectInstance));

        invocation->do_clear_color(true);
        invocation->draw(2, 1, 0, 0);
        // probe("after red, red spot", 75, 75);
        // probe("after red, green spot", 175, 75);

        // green: instance 1
        rect = { // green
            glm::translate(glm::mat4(1.0f), glm::vec3(150.0f, 50.0f, 0.0f))
                * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 1.0f)),
            glm::vec4(0, 1, 0, 1)
        };
        instance_buffer->upload_data(&rect, sizeof(RectInstance));

        invocation->do_clear_color(false);
        invocation->draw(2, 1, 0, 0);
        // probe("after green, red spot", 75, 75);
        // probe("after green, green spot", 175, 75);

        device.present(*window);
        gl_device.gl_get_error();

        std::cout << "FRAME END\n" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

inline void test_sampler_array() {
    engine::graphics::opengl::GLGraphicsDevice gl_device;
    engine::graphics::GraphicsDevice& device = gl_device;
    auto window = device.create_window(800, 600, "test window");
    uint32_t window_width = window->get_width();
    uint32_t window_height = window->get_height();
    auto window_render_target = device.create_window_render_target(*window);

    auto program = create_rendering_program(
        device,
        fs::current_path() / "res/shaders/testing/texture_array.vert", 
        fs::current_path() / "res/shaders/testing/texture_array.frag",
        engine::graphics::PrimitiveTopology::TriangleList
    );

    auto texture_image = engine::asset::Image::load_image(fs::current_path() / "res/textures/astolfo.png");
    auto texture = device.create_texture(
        engine::graphics::TextureDimension::D2,
        engine::graphics::TextureFormat::RGBA32F,
        texture_image.get_width(),
        texture_image.get_height(),
        1,  // depth
        1   // mip levels
    );
    texture->upload_image(texture_image);

    auto sampler = device.create_sampler();

    auto invocation = program->create_invocation();
    invocation->bind_render_target(*window_render_target);
    invocation->bind_sampled_texture(
        {0, 1},
        *texture,
        *sampler
    );
    invocation->validate();

    while(true) {
        invocation->draw(1);

        device.present(*window);
        dynamic_cast<engine::graphics::opengl::GLGraphicsDevice&>(device).gl_get_error();
        std::this_thread::sleep_for(std::chrono::nanoseconds(1000000000 / 60));
    }
}