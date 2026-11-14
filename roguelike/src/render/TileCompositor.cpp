#include "TileCompositor.h"

TileCompositor::TileCompositor(
    GraphicsDevice& device,
    const Tileset& tileset,
    const TileGrid& grid
) : device(device),
    tileset(tileset),
    grid(grid)
{
    tex_width = tileset.get_tile_size() * grid.get_width();
    tex_height = tileset.get_tile_size() * grid.get_height();
    assert(tex_width != 0);
    assert(tex_height != 0);
}

// creates a new texture that represents the given layer at the given channel
std::unique_ptr<engine::graphics::Texture> TileCompositor::create_texture(uint64_t layer_id, std::string channel) {
    using namespace engine::graphics;
    using namespace engine::tile;
    using namespace engine::math;
    using namespace engine::asset;
    
    // create texture + target rendering to texture
    auto texture = device.create_texture(
        TextureDimension::D2, TextureFormat::RGBA32F, 
        tex_width, tex_height, 1, 1
    );
    auto target = device.create_render_target(
        tex_width, tex_height
    );
    target->bind_texture(0, *texture);
    target->validate();

    // render tiles
    Renderer2D renderer(device, *target);
    uint64_t tile_size = tileset.get_tile_size();
    auto render_tile = [&](TileID tile_id, uint64_t x, uint64_t y) {
        // compute coordinates to render to on the texture
        uint64_t px_x = x * tile_size;
        uint64_t px_y = y * tile_size;
        assert(px_x + tile_size <= tex_width);
        assert(px_y + tile_size <= tex_height);

        // get image from tile atlas
        const Tile& tile = tileset.get_tile(tile_id);
        const TileAtlas& atlas = tile.atlas;
        const Image& atlas_image = atlas.get_channel(channel);

        // figure out tile UV within atlas
        glm::vec2 tl = (glm::vec2) tile.pos;
        glm::vec2 br = (glm::vec2) (tile.pos + glm::ivec2 {tile_size, tile_size});
        tl.x /= atlas_image.get_width(), tl.y /= atlas_image.get_height();
        br.x /= atlas_image.get_width(), br.y /= atlas_image.get_height();

        // draw tile
        renderer.draw_rect(
            Rect2 {{px_x, px_y}, {tile_size, tile_size}},
            get_texture(atlas_image),
            glm::vec4(1),
            Rect2 {tl, br - tl}
        );
    };
    const TileLayer& layer = grid.get_layer(layer_id);
    for(uint64_t x = 0; x < grid.get_width(); x++) {
        for(uint64_t y = 0; y < grid.get_height(); y++) {
            const std::vector<TileID>& tiles = layer.get({x, y});
            for(TileID tile_id : tiles) {
                render_tile(tile_id, x, y);
            }
        }
    }
    renderer.render();

    return texture;
}

engine::graphics::Texture& TileCompositor::get_texture(const engine::asset::Image& tile_image) {
    using namespace engine::graphics;
    auto asset_id = tile_image.get_asset_id();
    if(tile_textures.contains(asset_id)) {
        return *tile_textures.at(asset_id);
    }

    // create texture
    auto texture = device.create_texture(
        TextureDimension::D2, TextureFormat::RGBA32F, 
        tile_image.get_width(), tile_image.get_height(), 1, 1
    );
    texture->upload_image(tile_image);
    tile_textures.emplace(asset_id, std::move(texture));
    return *tile_textures.at(asset_id);
}