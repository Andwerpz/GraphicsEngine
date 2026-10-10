#pragma once

#include <engine/ui/UIRenderer.h>
#include <glad/glad.h>

#include <engine/assets/Font.h>

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
            auto draw_bordered_rect = [&ui_renderer](
                float xoff, float yoff,
                float width, float height
            ) -> void {
                ui_renderer.draw_rect(
                    {xoff, yoff, width, height}, {0.1, 0.1, 0.1, 1.0}
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

            ui_renderer.draw_rect({50, 50, 50, 50}, {1, 0, 0, 1});
            ui_renderer.draw_rect({150, 50, 50, 50}, {0, 1, 0, 1});
            ui_renderer.draw_rect({250, 50, 50, 50}, {0, 0, 1, 1});

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
            
            ui_renderer.render();

            device.present(*window);
            dynamic_cast<engine::graphics::opengl::GLGraphicsDevice&>(device).gl_get_error();
        },
        std::chrono::milliseconds(1000 / 60)
    );

    application.run();
}

inline void test_font() {
    fs::path font_path = fs::current_path() / "res/fonts/Consolas-Regular.ttf";
    auto font = engine::assets::Font::load_font(font_path);
}