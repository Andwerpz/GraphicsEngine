#pragma once

#include <engine/ui/UIRenderer.h>
#include <engine/ui/UIContext.h>
#include <engine/ui/node/Container.h>
#include <engine/ui/node/Label.h>
#include <engine/ui/node/Button.h>
#include <engine/ui/node/TextButton.h>
#include <engine/ui/node/Viewport.h>
#include <engine/ui/node/Window.h>
#include <glad/glad.h>

#include <engine/asset/Font.h>

#include "utils.h"

inline void test_ui_renderer() {
    engine::graphics::opengl::GLGraphicsDevice device;
    auto window = device.create_window(800, 600, "test window");
    uint32_t window_width = window->get_width();
    uint32_t window_height = window->get_height();
    auto window_render_target = device.create_window_render_target(*window);

    engine::ui::UIRenderer ui_renderer(
        device,
        *window_render_target
    );

    auto font = engine::asset::Font::load("res/fonts/Consolas-Regular.ttf", 48.0f);
    font.get_atlas().export_as_png(fs::current_path() / "atlas.png");

    engine::app::Application application;
    auto& task_loop = application.get_task_loop();
    task_loop.add_periodic_task(
        "render_ui",
        [&]() -> void {
            using namespace engine::graphics;
            window_render_target->clear(ClearFlags::Color);

            auto draw_bordered_rect = [&ui_renderer](
                float xoff, float yoff,
                float width, float height
            ) -> void {
                ui_renderer.draw_rect(
                    {{xoff, yoff}, {width, height}}, {0.1, 0.1, 0.1, 1.0}
                );

                float xmin = xoff, ymin = yoff;
                float xmax = xoff + width - 1, ymax = yoff + height - 1;
                ui_renderer.draw_line(
                    {xmin, ymin}, {xmax, ymin},
                    1, {0.3, 0.3, 0.3, 1.0}
                );
                ui_renderer.draw_line(
                    {xmin, ymin}, {xmin, ymax},
                    1, {0.3, 0.3, 0.3, 1.0}
                );
                ui_renderer.draw_line(
                    {xmin, ymax}, {xmax, ymax},
                    1, {0.3, 0.3, 0.3, 1.0}
                );
                ui_renderer.draw_line(
                    {xmax, ymin}, {xmax, ymax},
                    1, {0.3, 0.3, 0.3, 1.0}
                );

                ui_renderer.draw_line(
                    {xmin, ymax + 2}, {xmax, ymax + 2},
                    1, {1, 1, 1, 1}
                );

                ui_renderer.draw_line(
                    {xmin, ymin}, {xmax, ymax},
                    5, {1, 1, 1, 1}
                );
            };

            ui_renderer.draw_rect({{50, 50}, {50, 50}}, {1, 0, 0, 1});
            ui_renderer.draw_rect({{150, 50}, {50, 50}}, {0, 1, 0, 1});
            ui_renderer.draw_rect({{250, 50}, {50, 50}}, {0, 0, 1, 1});

            for(int i = 1; i <= 10; i++) {
                ui_renderer.draw_line(
                    {50 + i * 30, 200}, {100 + i * 30, 300}, 
                    i, {1, 1, 1, 1}
                );
            }

            draw_bordered_rect(305, 50, 50, 50);
            draw_bordered_rect(500, 50, 100, 200);

            const int N = 20;
            for(int i = 0; i < N; i++) {
                float sx = 200, sy = 400;
                float ex = sx + sin(3.14159265 * 2.0 / N * i) * 100;
                float ey = sy + cos(3.14159265 * 2.0 / N * i) * 100;
                ui_renderer.draw_line({sx, sy}, {ex, ey}, 1, {1, 1, 1, 1});
            }

            ui_renderer.draw_text("holy moly, is this working?", {50, 550}, font);
            
            ui_renderer.render();

            device.present(*window);
            dynamic_cast<engine::graphics::opengl::GLGraphicsDevice&>(device).gl_get_error();
        },
        std::chrono::milliseconds(1000 / 60)
    );

    application.run();
}

inline void test_load() {
    auto font = engine::asset::Font::load("res/fonts/Consolas-Regular.ttf", 24.0f);
    auto& atlas = font.get_atlas();
    atlas.export_as_png(fs::current_path() / "atlas.png");
}

inline void test_ui_context() {
    engine::graphics::opengl::GLGraphicsDevice gl_device;
    engine::graphics::GraphicsDevice& device = gl_device;
    auto window = device.create_window(800, 600, "test window");
    uint32_t window_width = window->get_width();
    uint32_t window_height = window->get_height();
    auto window_render_target = device.create_window_render_target(*window);

    engine::ui::UIContext ui_context(device, *window_render_target);
    engine::ui::Container& root_node = ui_context.get_root();
    root_node.emplace_child<engine::ui::Label>(
        "abcdefgh",
        "res/fonts/Consolas-Regular.ttf", 32.0,
        engine::ui::UIRect {{5, 5}, {200, 50}}
    );
    root_node.emplace_child<engine::ui::Button>(
        [&]() -> void {
            std::cout << "button clicked" << std::endl;
        },
        engine::ui::UIRect {{5, 60}, {200, 50}}
    );
    root_node.emplace_child<engine::ui::TextButton>(
        "text button",
        "res/fonts/Consolas-Regular.ttf", 24.0,
        [&]() -> void {
            std::cout << "text button clicked" << std::endl;
        },
        engine::ui::UIRect {{5, 115}, {200, 50}}
    );
    auto& ui_window2 = root_node.emplace_child<engine::ui::Window>(
        "sigma 16",
        glm::vec2 {55, 220}, glm::vec2 {300, 300}
    );
    auto& ui_window3 = root_node.emplace_child<engine::ui::Window>(
        "sigma 24",
        glm::vec2 {55, 220}, glm::vec2 {300, 300}
    );

    ui_window2.do_background_blur(true);
    ui_window3.do_background_blur(true);
    ui_window2.set_blur_sigma(16);
    ui_window3.set_blur_sigma(24);

    engine::app::Application application;

    auto& input_handler = application.get_input_handler();
    input_handler.add_window(*window);
    input_handler.set_cursor_position_callback(
        *window, 
        [&](const engine::app::CursorPositionEvent& ev) -> void {
            ui_context.on_cursor_position(ev);
        }
    );
    input_handler.set_mouse_button_callback(
        *window,
        [&](const engine::app::MouseButtonEvent& ev) -> void {
            ui_context.on_mouse_button(ev);
        }
    );
    input_handler.set_key_callback(
        *window,
        [&](const engine::app::KeyEvent& ev) -> void {
            ui_context.on_key(ev);
        }
    );
    input_handler.set_scroll_callback(
        *window,
        [&](const engine::app::ScrollEvent& ev) -> void {
            ui_context.on_scroll(ev);
        }
    );

    auto& task_loop = application.get_task_loop();
    task_loop.add_periodic_task(
        "render_ui",
        [&]() -> void {
            using namespace engine::graphics;
            window_render_target->clear(ClearFlags::Color);

            ui_context.update();
            ui_context.render();

            device.present(*window);
            dynamic_cast<engine::graphics::opengl::GLGraphicsDevice&>(device).gl_get_error();
        },
        std::chrono::milliseconds(1000 / 120)
    );


    fs::path vertex_path = fs::current_path() / "res/shaders/testing/uv_screen.vert";
    fs::path fragment_path = fs::current_path() / "res/shaders/testing/uv_screen.frag";
    auto program = create_rendering_program(
        device, 
        vertex_path, 
        fragment_path,
        engine::graphics::PrimitiveTopology::TriangleList
    );

    auto texture_image = engine::asset::Image::load("res/textures/sanat.png");
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

    // auto font = engine::asset::Font::load("engine://res/fonts/consolas.ttf", 12);
    // auto texture = device.create_texture(
    //     engine::graphics::TextureDimension::D2,
    //     engine::graphics::TextureFormat::RGBA32F,
    //     font.get_atlas().get_width(),
    //     font.get_atlas().get_height(),
    //     1,  // depth
    //     1   // mip levels
    // );
    // texture->upload_image(font.get_atlas());
    // auto sampler = device.create_sampler(
    //     engine::graphics::SamplerFilter::Nearest,
    //     engine::graphics::SamplerFilter::Nearest
    // );

    glm::vec2 texture_dim = {texture->get_width(), texture->get_height()};
    auto& ui_window = root_node.emplace_child<engine::ui::Window>(
        "texture",
        glm::vec2 {5, 170}, texture_dim
    );
    auto& viewport = ui_window.emplace_child<engine::ui::Viewport>(
        device,
        engine::ui::UIRect {{0, 0}, texture_dim}
    );

    auto invocation = program->create_invocation(); 
    invocation->bind_render_target(viewport.get_render_target());
    invocation->validate();
    invocation->bind_sampled_texture(
        {0, 0}, *texture, *sampler
    );

    task_loop.add_periodic_task(
        "render_viewport",
        [&]() -> void {
            invocation->draw(1);
            dynamic_cast<engine::graphics::opengl::GLGraphicsDevice&>(device).gl_get_error();
        },
        std::chrono::milliseconds(1000 / 60)
    );

    // auto cube_program = create_rendering_program(
    //     device,
    //     fs::current_path() / "res/shaders/testing/texture.vert", 
    //     fs::current_path() / "res/shaders/testing/texture.frag",
    //     engine::graphics::PrimitiveTopology::TriangleList
    // );
    // auto skybox_program = create_rendering_program(
    //     device, 
    //     fs::current_path() / "res/shaders/testing/skybox.vert", 
    //     fs::current_path() / "res/shaders/testing/skybox.frag",
    //     engine::graphics::PrimitiveTopology::TriangleList
    // );

    // auto cube_vertex_buffer = device.create_buffer(
    //     sizeof(cube_vertices),
    //     engine::graphics::BufferUsage::Vertex,
    //     engine::graphics::BufferFrequency::Static,
    //     cube_vertices
    // );
    // auto cube_normal_buffer = device.create_buffer(
    //     sizeof(cube_normals),
    //     engine::graphics::BufferUsage::Vertex,
    //     engine::graphics::BufferFrequency::Static,
    //     cube_normals
    // );
    // auto cube_uv_buffer = device.create_buffer(
    //     sizeof(cube_uvs),
    //     engine::graphics::BufferUsage::Vertex,
    //     engine::graphics::BufferFrequency::Static,
    //     cube_uvs
    // );

    // auto sampler = device.create_sampler();

    // auto texture_image = engine::asset::Image::load(fs::current_path() / "res/textures/astolfo.png");
    // auto texture = device.create_texture(
    //     engine::graphics::TextureDimension::D2,
    //     engine::graphics::TextureFormat::RGBA32F,
    //     texture_image.get_width(),
    //     texture_image.get_height(),
    //     1,  // depth
    //     1   // mip levels
    // );
    // texture->upload(texture_image);

    // auto** skybox_image = (engine::asset::Image**) malloc(sizeof(engine::asset::Image*) * 6);
    // const char* skybox_strs[] = {
    //     "right.jpg",   // +X
    //     "left.jpg",    // -X
    //     "top.jpg",     // +Y
    //     "bottom.jpg",  // -Y
    //     "front.jpg",   // +Z
    //     "back.jpg",    // -Z
    // };
    // for(int i = 0; i < 6; i++) {
    //     auto image = engine::asset::Image::load(fs::current_path() / "res/textures/lake" / skybox_strs[i]);
    //     skybox_image[i] = new engine::asset::Image(std::move(image));
    // }
    // auto skybox = device.create_texture(
    //     engine::graphics::TextureDimension::Cube,
    //     engine::graphics::TextureFormat::RGBA32F,
    //     skybox_image[0]->get_width(),
    //     skybox_image[0]->get_height(),
    //     1,
    //     1
    // );
    // for(int i = 0; i < 6; i++) {
    //     skybox->upload(*skybox_image[i], 0, i);
    // }

    // auto skybox_invocation = skybox_program->create_invocation(); 
    // {
    //     skybox_invocation->bind_render_target(viewport.get_render_target());

    //     skybox_invocation->bind_vertex_attribute(
    //         0,      // location
    //         *cube_vertex_buffer,
    //         0,      // offset
    //         12,     // stride
    //         engine::graphics::VertexInputRate::Vertex,
    //         engine::graphics::VertexFormat::Float32x3
    //     );
    //     skybox_invocation->bind_vertex_attribute(
    //         1,      // location
    //         *cube_normal_buffer,
    //         0,      // offset
    //         12,     // stride
    //         engine::graphics::VertexInputRate::Vertex,
    //         engine::graphics::VertexFormat::Float32x3
    //     );
    //     skybox_invocation->bind_vertex_attribute(
    //         2,      // location
    //         *cube_uv_buffer,
    //         0,      // offset
    //         8,      // stride
    //         engine::graphics::VertexInputRate::Vertex,
    //         engine::graphics::VertexFormat::Float32x2
    //     );

    //     skybox_invocation->bind_sampled_texture(
    //         {0, 1},
    //         *skybox,
    //         *sampler
    //     );

    //     skybox_invocation->validate();
    // }

    // auto cube_invocation = cube_program->create_invocation();
    // {
    //     cube_invocation->bind_render_target(viewport.get_render_target());
        
    //     cube_invocation->bind_vertex_attribute(
    //         0,      // location
    //         *cube_vertex_buffer,
    //         0,      // offset
    //         12,     // stride
    //         engine::graphics::VertexInputRate::Vertex,
    //         engine::graphics::VertexFormat::Float32x3
    //     );
    //     cube_invocation->bind_vertex_attribute(
    //         1,      // location
    //         *cube_normal_buffer,
    //         0,      // offset
    //         12,     // stride
    //         engine::graphics::VertexInputRate::Vertex,
    //         engine::graphics::VertexFormat::Float32x3
    //     );
    //     cube_invocation->bind_vertex_attribute(
    //         2,      // location
    //         *cube_uv_buffer,
    //         0,      // offset
    //         8,      // stride
    //         engine::graphics::VertexInputRate::Vertex,
    //         engine::graphics::VertexFormat::Float32x2
    //     );

    //     cube_invocation->bind_sampled_texture(
    //         {0, 1},
    //         *texture,
    //         *sampler
    //     );

    //     cube_invocation->validate();
    // }

    // auto skybox_projection = skybox_invocation->get_uniform<glm::mat4>("camera.projection");
    // auto skybox_view = skybox_invocation->get_uniform<glm::mat4>("camera.view");
    // auto cube_projection = cube_invocation->get_uniform<glm::mat4>("camera.projection");
    // auto cube_view = cube_invocation->get_uniform<glm::mat4>("camera.view");

    // skybox_invocation->do_clear_color(false);
    // skybox_invocation->do_clear_depth(true);
    // cube_invocation->do_clear_color(false);
    // cube_invocation->do_clear_depth(true);

    // task_loop.add_periodic_task(
    //     "render_viewport",
    //     [&]() -> void {
    //         float time = std::chrono::duration<float>(std::chrono::steady_clock::now().time_since_epoch()).count();
    //         float angle = time * 1.0f; // radians/sec
    //         float radius = 2.0f;
    //         float height = 1.0f;
    //         glm::vec3 camera_position(
    //             radius * std::cos(angle),
    //             height,
    //             radius * std::sin(angle)
    //         );

    //         glm::mat4 projection = glm::perspective(
    //             glm::radians(90.0f),
    //             1.0f,
    //             0.1f,
    //             100.0f
    //         );
    //         glm::mat4 view = glm::lookAt(
    //             camera_position,
    //             glm::vec3(0.0f),
    //             glm::vec3(0.0f, 1.0f, 0.0f)
    //         );

    //         window_render_target->clear(engine::graphics::ClearFlags::Color);

    //         skybox_invocation->set_uniform(skybox_projection, projection);
    //         skybox_invocation->set_uniform(skybox_view, view);
    //         skybox_invocation->draw(12);

    //         cube_invocation->set_uniform(cube_projection, projection);
    //         cube_invocation->set_uniform(cube_view, view);
    //         cube_invocation->draw(12);

    //         dynamic_cast<engine::graphics::opengl::GLGraphicsDevice&>(device).gl_get_error();
    //     },
    //     std::chrono::milliseconds(1000 / 120)
    // );

    application.run();
}