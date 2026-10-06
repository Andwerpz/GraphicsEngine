#pragma once
#include <engine/app/Application.h>
#include <engine/app/TaskLoop.h>
#include <engine/app/input/InputHandler.h>

#include "utils.h"

// make sure that we can render a moving scene via a application
inline void test_application() {
    engine::graphics::opengl::GLGraphicsDevice device;
    auto window = device.create_window(800, 600, "test window");
    uint32_t window_width = window->get_width();
    uint32_t window_height = window->get_height();

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
    
    auto window_render_target = device.create_window_render_target(*window);
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

    dynamic_cast<engine::graphics::opengl::GLGraphicsDevice&>(device).gl_get_error();

    bool mouse_pressed = false;
    float mouse_xpos, mouse_ypos;
    float yrot = 0.0;
    float xrot = 0.0;

    engine::app::Application application;
    auto& input_handler = application.get_input_handler();
    input_handler.add_window(*window);
    input_handler.set_cursor_position_callback(
        *window, 
        [&](const engine::app::CursorPositionEvent& ev) -> void {
            if(mouse_pressed) {
                float xdiff = ev.xpos - mouse_xpos;
                float ydiff = ev.ypos - mouse_ypos;
                xrot -= ydiff * 0.01;
                yrot -= xdiff * 0.01;
                xrot = glm::clamp(xrot, -glm::pi<float>() / 2.0f + 0.001f, glm::pi<float>() / 2.0f - 0.001f);
            }
            mouse_xpos = ev.xpos;
            mouse_ypos = ev.ypos;
        }
    );
    input_handler.set_mouse_button_callback(
        *window,
        [&](const engine::app::MouseButtonEvent& ev) -> void {
            if(ev.button == engine::app::MouseButton::Button1) {
                mouse_pressed = ev.action == engine::app::MouseButtonAction::Press;
            }
        }
    );
    input_handler.set_key_callback(
        *window,
        [&](const engine::app::KeyEvent& ev) -> void {
            std::cout << "Key event" << std::endl;
        }
    );

    auto& task_loop = application.get_task_loop();
    task_loop.add_periodic_task(
        "render_cube",
        [&]() -> void {
            float time = std::chrono::duration<float>(std::chrono::steady_clock::now().time_since_epoch()).count();
            float angle = 1.0f; // radians/sec
            float radius = 2.0f;
            float height = 1.0f;
            glm::vec3 camera_position(
                radius * std::cos(angle),
                height,
                radius * std::sin(angle)
            );

            glm::mat4 rotation(1.0f);
            rotation = glm::rotate(rotation, yrot, glm::vec3(0, 1, 0));
            rotation = glm::rotate(rotation, xrot, glm::vec3(1, 0, 0));
            glm::vec3 forward = glm::vec3(rotation * glm::vec4(0, 0, -1, 0));

            glm::mat4 projection = glm::perspective(
                glm::radians(90.0f),
                window_width / static_cast<float>(window_height),
                0.1f,
                100.0f
            );
            glm::mat4 view = glm::lookAt(
                camera_position,
                camera_position + forward,
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
            invocation->set_uniform(projection_token, projection);
            invocation->set_uniform(view_token, view);
            invocation->draw(36);

            device.present(*window);

            dynamic_cast<engine::graphics::opengl::GLGraphicsDevice&>(device).gl_get_error();
        },
        std::chrono::milliseconds(1000 / 60)
    );

    application.run();
}