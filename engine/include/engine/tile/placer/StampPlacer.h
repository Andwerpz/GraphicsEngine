#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <engine/tile/placer/TilePlacer.h>
#include <engine/tile/TileGrid.h>

namespace engine::tile {

// a StampPlacer knows how to place a predefined list of tiles at predefined offsets about an anchor.

class StampPlacer : public TilePlacer {
public:
    ~StampPlacer() override = default;

    struct Entry {
        TileID tile;
        glm::ivec2 offset;
    };

    StampPlacer(
        std::string name, 
        std::vector<Entry>&& _entries
    ) : TilePlacer(name),
        entries(std::move(_entries)) 
    {
        // should have at least one entry
        assert(entries.size() != 0);
    }

    void place_stamp(TileLayer& layer, glm::ivec2 anchor) const {
        for(const auto& e : entries) {
            layer.push(anchor + e.offset, e.tile);
        }
    }

private:
    std::vector<Entry> entries;
};

}