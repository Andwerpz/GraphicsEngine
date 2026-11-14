#include <filesystem>
namespace fs = std::filesystem;

#include <iostream>
#include <chrono>
#include <thread>

#include <engine/asset/Font.h>
#include <engine/asset/Image.h>

#include <engine/graphics/opengl/GLGraphicsDevice.h>
#include <engine/graphics/GraphicsDevice.h>
#include <engine/graphics/Shader.h>
#include <engine/graphics/Buffer.h>

#include <engine/app/Application.h>
#include <engine/app/TaskLoop.h>
#include <engine/app/input/InputHandler.h>

#include <engine/ui/UIRenderer.h>
#include <engine/ui/UIContext.h>
#include <engine/ui/widget/TextButton.h>
#include <engine/ui/widget/Viewport.h>
#include <engine/ui/widget/Window.h>
#include <engine/ui/widget/TextField.h>
#include <engine/ui/widget/ScrollContainer.h>

#include <engine/tile/Tileset.h>
#include <engine/tile/CellSet.h>
#include <engine/tile/TileGrid.h>
#include <engine/tile/placer/RandomPlacer.h>
#include <engine/tile/placer/AutotilePlacer.h>
#include <engine/tile/placer/StampPlacer.h>

#include "render/TileCompositor.h"

int main() {
    // setup graphics device
    engine::graphics::opengl::GLGraphicsDevice gl_device;
    engine::graphics::GraphicsDevice& device = gl_device;
    auto window = device.create_window(800, 600, "test window");
    uint32_t window_width = window->get_width();
    uint32_t window_height = window->get_height();
    auto window_render_target = device.create_window_render_target(*window);

    // setup UI context
    engine::ui::UIContext ui_context(device, *window_render_target);
    engine::ui::Container& context_root = ui_context.get_root();

    // testing windows
    {
        auto& ui_window1 = context_root.emplace_child<engine::ui::Window>(
            "sigma 16 + really long title that should really get clipped but maybe isn't getting clipped",
            glm::vec2 {55, 220}, glm::vec2 {300, 300}
        );
        auto& ui_window2 = context_root.emplace_child<engine::ui::Window>(
            "sigma 24 + really long title that should not get clipped when this window stretches out sufficiently large. How is your day going by the way, mine is going fine, got some chipotle and stuff and was thinking about eating it soon",
            glm::vec2 {55, 220}, glm::vec2 {300, 300}
        );
        ui_window1.do_background_blur(true);
        ui_window2.do_background_blur(true);
        ui_window1.set_blur_sigma(16);
        ui_window2.set_blur_sigma(24);

        auto& scroll_container = ui_window1.emplace_child<engine::ui::ScrollContainer>();
        scroll_container.set_anchor(glm::vec2(0), glm::vec2(1));
        scroll_container.set_content_dim(glm::vec2(400));
        scroll_container.do_vertical_scrolling(true);
        // scroll_container.do_horizontal_scrolling(true);

        // auto& background_rect = scroll_container.emplace_child<engine::ui::Rect>();
        // background_rect.set_anchor(glm::vec2(0), glm::vec2(1));
        // background_rect.set_outline_color(glm::vec4(1));
        // background_rect.set_outline_inset(1);
        // background_rect.do_draw_outline(true);

        auto& label = scroll_container.emplace_child<engine::ui::Label>(
            "abcdefgh",
            "res/font/consolas.ttf", 32.0
        );
        label.set_offset({5, 5});
        label.set_fixed_dim({200, 50});
        auto& text_button = scroll_container.emplace_child<engine::ui::TextButton>(
            [&]() -> void {
                std::cout << "text button clicked" << std::endl;
            },
            "text button",
            "res/font/consolas.ttf", 24.0
        );
        text_button.set_offset({5, 60});
        text_button.set_fixed_dim({200, 50});

        auto& text_field = scroll_container.emplace_child<engine::ui::TextField>();
        text_field.set_offset({5, 115});
        text_field.set_fixed_dim({200, 20});
        text_field.set_input_changed_callback(
            [](const std::string& input) -> void { 
                std::cout << "text input : " << input << std::endl; 
            }
        );
    }

    // tileset editor window
    std::unique_ptr<engine::graphics::Texture> floor_albedo_tex;
    std::unique_ptr<engine::graphics::Texture> floor_normal_tex;
    {
        using TileID = engine::tile::TileID;
        using Tileset = engine::tile::Tileset;
        using CellSet = engine::tile::CellSet;
        using TileGrid = engine::tile::TileGrid;
        using TileLayer = engine::tile::TileLayer;

        using RandomPlacer = engine::tile::RandomPlacer;
        using AutotilePlacer = engine::tile::AutotilePlacer;
        using StampPlacer = engine::tile::StampPlacer;

        Tileset tileset = Tileset::load("res/tileset/tileset.jsonc");
        std::cout << "LOADED TILESET : " << tileset.get_name() << std::endl;
        std::cout << std::endl;

        auto atlas_names = tileset.get_atlas_names();
        auto tile_names = tileset.get_tile_names();
        auto placer_names = tileset.get_placer_names();

        std::cout << "ATLASES : \n";
        for(auto x : atlas_names) std::cout << x << "\n";
        std::cout << std::endl;

        std::cout << "TILES : \n";
        for(auto x : tile_names) std::cout << x << "\n";
        std::cout << std::endl;

        std::cout << "PLACERS : \n";
        for(auto x : placer_names) std::cout << x << "\n";
        std::cout << std::endl;


        constexpr uint64_t LAYER_FLOOR = 0;
        constexpr uint64_t LAYER_WALL = 1;
        constexpr uint64_t LAYER_TRIM = 2;

        CellSet floor_cells = CellSet::from_rect(glm::vec2(0), glm::vec2(5));
        TileGrid grid(10, 10);
        
        // place floor
        {
            const RandomPlacer& floor_placer = tileset.get_placer<RandomPlacer>("dev.floor");
            floor_placer.place_random(grid.get_layer(LAYER_FLOOR), floor_cells);

            TileLayer& floor_layer = grid.get_layer(LAYER_FLOOR);
            for(auto c : floor_cells) {
                assert(floor_layer.get(c).size() == 1);
                TileID id = floor_layer.get(c)[0];
                std::cout << "TILE : " << c.x << " " << c.y << " " << id << std::endl;
            }
        }

        // render world to texture
        TileCompositor compositor(device, tileset, grid);
        floor_albedo_tex = compositor.create_texture(LAYER_FLOOR, "albedo");
        floor_normal_tex = compositor.create_texture(LAYER_FLOOR, "normal");
    }

    // setup application
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

    // run application
    application.run();

    return 0;
}