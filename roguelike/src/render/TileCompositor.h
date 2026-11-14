#pragma once
#include <engine/graphics/GraphicsDevice.h>
#include <engine/graphics/Texture.h>

#include <engine/tile/TileGrid.h>
#include <engine/tile/TileAtlas.h>
#include <engine/tile/Tileset.h>

#include <engine/asset/EmbeddedResource.h>
#include <engine/asset/Image.h>
#include <engine/render/Renderer2D.h>

// knows how to take in a Tileset and a TileGrid generated using that tileset
// and output textures that correspond with the generated tile grid

class TileCompositor {
    using GraphicsDevice = engine::graphics::GraphicsDevice;
    using Texture = engine::graphics::Texture;
    using Renderer2D = engine::render::Renderer2D;

    using TileID = engine::tile::TileID;
    using Tileset = engine::tile::Tileset;
    using TileGrid = engine::tile::TileGrid;
    using TileLayer = engine::tile::TileLayer;
    using TileAtlas = engine::tile::TileAtlas;

public:
    TileCompositor(
        GraphicsDevice& device,
        const Tileset& tileset,
        const TileGrid& grid
    );

    // creates a new texture that represents the given layer at the given channel
    std::unique_ptr<Texture> create_texture(
        uint64_t layer,
        std::string channel
    );

private:
    // returns the texture that represents the given image
    // this does caching.
    Texture& get_texture(const engine::asset::Image& tile_image);

    GraphicsDevice& device;
    const Tileset& tileset;
    const TileGrid& grid;

    // dimensions of textures produced by this compositing pipeline
    uint64_t tex_width, tex_height;

    // cached tile atlas textures
    std::map<engine::asset::AssetID, std::unique_ptr<Texture>> tile_textures;
};