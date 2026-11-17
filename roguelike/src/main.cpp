#include <filesystem>
namespace fs = std::filesystem;

#include <iostream>
#include <chrono>
#include <thread>
#include <glm/glm.hpp>

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
#include "render/CubeSet.h"

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

        constexpr uint32_t grid_w = 18;
        constexpr uint32_t grid_h = 16;
        CellSet walkable_cells;
        walkable_cells.insert_rect({2, 3}, {6, 12});
        walkable_cells.insert_rect({6, 6}, {10, 8});
        walkable_cells.insert_rect({10, 4}, {13, 10});
        walkable_cells.insert_rect({13, 6}, {16, 10});
        walkable_cells.insert_rect({3, 12}, {8, 15});
        CellSet all_cells = CellSet::from_rect({0, 0}, {grid_w, grid_h});
        TileGrid grid(grid_w, grid_h);
        
        // place floor
        {
            // floor cells are exactly the walkable cells
            CellSet floor_cells = walkable_cells;

            // place small tiles
            const RandomPlacer& floor_placer = tileset.get_placer<RandomPlacer>("dev.floor");
            floor_placer.place_random(grid.get_layer(LAYER_FLOOR), floor_cells);

            // place big tile
            const StampPlacer& big_tile = tileset.get_placer<StampPlacer>("dev.big_tile");
            big_tile.place_stamp(grid.get_layer(LAYER_FLOOR), {11, 7});
        }

        // place walls
        {
            // wall cells are any cell that is one or two above a walkable cell with no walkable cell above it
            CellSet above_walkable = walkable_cells.translated({0, -1}) - walkable_cells;
            above_walkable |= above_walkable.translated({0, -1});

            // place walls
            const RandomPlacer& wall_placer = tileset.get_placer<RandomPlacer>("dev.wall");
            wall_placer.place_random(grid.get_layer(LAYER_WALL), above_walkable);
        }

        // place trim
        {   
            // trim predicate is the walkable cells shifted upwards by 2
            // set of trim cells is all cells - trim predicate
            CellSet trim_predicate = walkable_cells.translated({0, -2});
            CellSet trim_cells = all_cells - trim_predicate;

            // place trim tiles
            const AutotilePlacer& trim_placer = tileset.get_placer<AutotilePlacer>("dev.trim");
            trim_placer.place_autotile(grid.get_layer(LAYER_TRIM), trim_cells, trim_predicate);
        }

        std::map<uint64_t, std::string> layer_to_string;
        layer_to_string[LAYER_FLOOR] = "floor";
        layer_to_string[LAYER_WALL] = "wall";
        layer_to_string[LAYER_TRIM] = "trim";
        auto render_layer = [&](uint64_t layer) -> void {
            // render world to texture
            TileCompositor compositor(device, tileset, grid);
            std::set<std::string> channels = tileset.get_channel_names();
            for(const std::string& channel : channels) {
                auto tex = compositor.create_texture(layer, channel);
                fs::path image_path = fs::current_path() / (layer_to_string[layer] + "_" + channel + ".png");
                tex->save_image().export_as_png(image_path);
                std::cout << "SAVED " << layer_to_string[layer] << " " << channel << " : " << image_path << std::endl;
            }
        };
        render_layer(LAYER_FLOOR);
        render_layer(LAYER_WALL);
        render_layer(LAYER_TRIM);

        // generate position texture
        {   
            CubeSet cubes;
            for(const auto& cell : walkable_cells) {
                cubes.insert({cell.x, -1, cell.y});
            }
            CellSet wall_cells = walkable_cells.dilated() - walkable_cells;
            for(const auto& cell : wall_cells) {
                cubes.insert({cell.x, 0, cell.y});
                cubes.insert({cell.x, 1, cell.y});
            }

            auto position_tex = cubes.create_texture(
                device, 
                grid_w, 
                grid_h, 
                tileset.get_tile_size(),
                CubeSet::PositionEncoding::Normalized
            );
            fs::path image_path = fs::current_path() / "position.png";
            position_tex->save_image().export_as_png(image_path);
            std::cout << "SAVED position : " << image_path << std::endl;
        }
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