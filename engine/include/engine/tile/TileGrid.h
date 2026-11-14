#pragma once
#include <map>
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include <engine/util/error.h>
#include <engine/util/macro.h>
#include <engine/tile/TileID.h>

namespace engine::tile {

class TileGrid;

// a TileLayer represents a 2D grid of concrete tiles
// each cell in the layer can have a stack of tiles placed onto them
class TileLayer {
    friend class TileGrid;

public:
    std::vector<TileID>& get(glm::ivec2 pos) {
        if(pos.x < 0 || pos.y < 0 || pos.x >= width || pos.y >= height) {
            engine::util::throw_error("pos out of range");
        }
        return layer[pos.x * height + pos.y];
    }
    const std::vector<TileID>& get(glm::ivec2 pos) const {
        if(pos.x < 0 || pos.y < 0 || pos.x >= width || pos.y >= height) {
            engine::util::throw_error("pos out of range");
        }
        return layer[pos.x * height + pos.y];
    }

    void set(glm::ivec2 pos, TileID tile) {
        clear(pos);
        push(pos, tile);
    }
    void push(glm::ivec2 pos, TileID tile) {
        auto& cell = get(pos);
        cell.push_back(tile);
    }
    void pop(glm::ivec2 pos) {
        auto& cell = get(pos);
        cell.pop_back();
    }
    void clear(glm::ivec2 pos) {
        auto& cell = get(pos);
        while(cell.size() != 0) cell.pop_back();
    }

private:
    TileLayer(uint64_t width, uint64_t height) :
        width(width), height(height), layer(width * height, std::vector<TileID>(0))
    {}

    NON_COPYABLE(TileLayer);

    // moveable by TileGrid
    TileLayer(TileLayer&&) = default;
    TileLayer& operator=(TileLayer&&) = default;

    uint64_t width, height;
    std::vector<std::vector<TileID>> layer;
};

// a TileGrid is a set of TileLayers keyed by user-provided constants
// all layers in a grid have the same dimensions
class TileGrid {
public:
    TileGrid(uint64_t width, uint64_t height) :
        width(width), height(height) {}
    NON_COPYABLE_NON_MOVABLE(TileGrid);

    uint64_t get_width() const {
        return width;
    }
    uint64_t get_height() const {
        return height;
    }

    bool contains_layer(uint64_t id) {
        return layers.contains(id);
    }

    // returns the layer referred to by the given id
    // if the layer does not exist, this creates it
    TileLayer& get_layer(uint64_t id) {
        if(!layers.contains(id)) {
            layers.emplace(id, std::unique_ptr<TileLayer>(new TileLayer(width, height)));
        }
        return *layers.at(id);
    }

    // const version of get_layer, does not create a non-existing layer
    const TileLayer& get_layer(uint64_t id) const {
        if(!layers.contains(id)) {
            engine::util::throw_error("Grid does not contain layer : " + std::to_string(id));
        }
        return *layers.at(id);
    }

private:
    // dimensions of all layers owned by this grid
    uint64_t width, height;

    // mapping of id to layer
    std::map<uint64_t, std::unique_ptr<TileLayer>> layers;
};

}