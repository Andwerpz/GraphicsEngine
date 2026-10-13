#pragma once

#include <engine/ui/UIRenderer.h>
#include <engine/ui/UIContext.h>
#include <engine/ui/node/Container.h>
#include <engine/ui/node/Label.h>
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

    fs::path font_path = fs::current_path() / "res/fonts/Consolas-Regular.ttf";
    auto font = engine::asset::Font::load_font(font_path, 48.0f);
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

inline void test_load_font() {
    fs::path font_path = fs::current_path() / "res/fonts/Consolas-Regular.ttf";
    auto font = engine::asset::Font::load_font(font_path, 32.0f);
    auto& atlas = font.get_atlas();
    atlas.export_as_png(fs::current_path() / "atlas.png");
}

inline void test_ui_context() {
    engine::graphics::opengl::GLGraphicsDevice device;
    auto window = device.create_window(800, 600, "test window");
    uint32_t window_width = window->get_width();
    uint32_t window_height = window->get_height();
    auto window_render_target = device.create_window_render_target(*window);

    fs::path font_path = fs::current_path() / "res/fonts/Consolas-Regular.ttf";
    auto font = engine::asset::Font::load_font(font_path, 32.0f);
    font.get_atlas().export_as_png(fs::current_path() / "atlas.png");

    engine::ui::UIContext ui_context(device, *window_render_target);
    engine::ui::Container& root_node = ui_context.get_root();
    root_node.emplace_child<engine::ui::Label>(
        "abcdefghijklmnopqrstuvwxyz",
        font,
        engine::ui::UIRect {{100, 100}, {300, 100}}
    );

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
        std::chrono::milliseconds(1000 / 60)
    );

    application.run();
}