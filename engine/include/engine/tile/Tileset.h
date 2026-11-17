#pragma once
#include <map>
#include <set>
#include <glm/glm.hpp>
#include <engine/asset/Asset.h>
#include <engine/asset/Image.h>
#include <engine/util/error.h>
#include <engine/util/macro.h>
#include <engine/tile/TileID.h>
#include <engine/tile/TileAtlas.h>
#include <engine/tile/placer/TilePlacer.h>

namespace engine::tile {

// a Tile is a container for all the stuff describing how to render it
// it has no idea how it should be placed with other tiles
struct Tile {
    // ids of tiles should be unique within a Tileset
    // these are assigned at runtime
    TileID id;

    // tile names should be unique within a tileset
    std::string name;

    // atlas that this tile comes from
    // this should refer to an atlas owned by its owning Tileset
    const TileAtlas& atlas;

    // top left of region in atlas
    glm::ivec2 pos;
};

// a Tileset is the container for all tile information
class Tileset : public engine::asset::Asset {
    friend class TilesetBuilder;

public:
    // loads a tileset from the filesystem
    static Tileset load(std::string_view addr);

    void export_tileset(std::string_view addr);

    const std::string& get_name() const {
        return name;
    }
    uint64_t get_tile_size() const {
        return tile_size;
    }

    const Tile& get_tile(std::string name) const {
        if(!tiles_by_name.contains(name)) {
            engine::util::throw_error("no tile exists with name : " + name);
        }
        return tiles.at(tiles_by_name.at(name));
    }
    const Tile& get_tile(TileID id) const {
        if(id >= tiles.size()) {
            engine::util::throw_error("TileID out of range");
        }
        return tiles.at(id);
    }

    template<class T>
    const T& get_placer(std::string name) const {
        auto it = placers.find(name);
        if(it == placers.end()) {
            engine::util::throw_error("placer : " + name + " does not exist");
        }
        const T* p = dynamic_cast<const T*>(it->second.get());
        if(!p) {
            engine::util::throw_error("placer : " + name + " is not a " + typeid(T).name());
        }
        return *p;
    }

    std::set<std::string> get_atlas_names() const {
        std::set<std::string> res;
        for(const auto& i : atlases) {
            res.emplace(i.first);
        }
        return res;
    }
    std::set<std::string> get_tile_names() const {
        std::set<std::string> res;
        for(const auto& i : tiles) {
            assert(!res.contains(i.name));
            res.emplace(i.name);
        }
        return res;
    }
    std::set<std::string> get_placer_names() const {
        std::set<std::string> res;
        for(const auto& i : placers) {
            res.emplace(i.first);
        }
        return res;
    }
    const std::set<std::string>& get_channel_names() const {
        return channel_names;
    }

private:
    Tileset(
        std::string_view addr,
        std::string name,
        uint64_t tile_size,
        std::map<std::string, std::unique_ptr<TileAtlas>>&& atlases,
        std::vector<Tile>&& tiles,
        std::map<std::string, std::unique_ptr<TilePlacer>>&& placers,
        std::map<std::string, TileID>&& tiles_by_name,
        std::set<std::string>&& channel_names
    ) : Asset(addr),
        name(name),
        tile_size(tile_size), 
        atlases(std::move(atlases)), 
        tiles(std::move(tiles)),
        placers(std::move(placers)),
        tiles_by_name(std::move(tiles_by_name)),
        channel_names(std::move(channel_names))
    {}
    NON_COPYABLE_NON_MOVABLE(Tileset);

    std::string name;

    // side length of every tile in pixels
    // every tile is a square
    uint64_t tile_size;

    // all atlases keyed by name
    std::map<std::string, std::unique_ptr<TileAtlas>> atlases;

    // all tiles keyed by id
    std::vector<Tile> tiles;

    // all tile placers keyed by name
    std::map<std::string, std::unique_ptr<TilePlacer>> placers;

    // mapping from tile name to tile id
    std::map<std::string, TileID> tiles_by_name;

    // set of all channel names
    // all atlases owned by this tileset should have exactly these channels
    std::set<std::string> channel_names;
};

}

