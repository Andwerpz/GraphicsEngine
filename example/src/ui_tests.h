#pragma once

#include <engine/ui/UIRenderer.h>
#include <glad/glad.h>

#include "utils.h"

inline void test_ui_rect() {
    engine::graphics::opengl::GLGraphicsDevice device;
    auto window = device.create_window(800, 600, "test window");
    uint32_t window_width = window->get_width();
    uint32_t window_height = window->get_height();
    auto window_render_target = device.create_window_render_target(*window);

    engine::ui::UIRenderer ui_renderer(
        device,
        *window_render_target
    );

    engine::app::Application application;
    auto& task_loop = application.get_task_loop();
    task_loop.add_periodic_task(
        "render_ui",
        [&]() -> void {
            ui_renderer.draw_rect({50, 50, 50, 50}, {1, 0, 0, 1});
            ui_renderer.draw_rect({150, 50, 50, 50}, {0, 1, 0, 1});
            ui_renderer.draw_rect({250, 50, 50, 50}, {0, 0, 1, 1});
            // ui_renderer.draw_rect({250, 50, 50, 50}, {1, 1, 1, 1});
            // ui_renderer.draw_rect({350, 50, 50, 50}, {1, 1, 1, 1});
            // ui_renderer.draw_rect({450, 50, 50, 50}, {1, 1, 1, 1});
            ui_renderer.render();

            device.present(*window);
            dynamic_cast<engine::graphics::opengl::GLGraphicsDevice&>(device).gl_get_error();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        },
        std::chrono::milliseconds(1000 / 60)
    );

    application.run();
}