#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <engine/tile/placer/TilePlacer.h>
#include <engine/tile/TileGrid.h>
#include <engine/tile/CellSet.h>

namespace engine::tile {

class AutotilePlacer : public TilePlacer {
public:
    ~AutotilePlacer() override = default;

    struct Neighbor {
        std::string name;
        glm::ivec2 offset;
    };
    enum class NeighborState {
        Any, 
        Present,
        Missing,
    };
    struct Rule {
        std::vector<NeighborState> when;
        TileID tile;
    };

    AutotilePlacer(
        std::string name,
        std::vector<Neighbor>&& _neighbors,
        std::vector<Rule>&& _rules
    ) : TilePlacer(name),
        neighbors(std::move(_neighbors)),
        rules(std::move(_rules)) 
    {
        // should have at least one rule
        assert(rules.size() != 0);

        // every rule must refer to all the neighbors
        for(const auto& r : rules) {
            assert(r.when.size() == neighbors.size());
        }

        // build LUT
        // TODO should probably not build LUT when neighbors is too large
        //   just fall back to iterating though all the rules for every tile
        assert(neighbors.size() <= 8);
        lut.resize(1u << neighbors.size());
        for(size_t i = 0; i < lut.size(); i++) {
            lut[i] = determine_tile(i);
        }

        // TODO make sure every single neighbor state has a corresponding output tile
        //   how to do this when the number of neighbors is very large? 
        //   maybe can do some linear algebra?
    }

    void place_autotile(TileLayer& layer, const CellSet& cells) const {
        for(const auto& c : cells) {
            // construct neighbor bitstring
            assert(neighbors.size() <= sizeof(uint64_t) * 8);
            uint64_t bits = 0;
            for(int i = (int) neighbors.size() - 1; i >= 0; i--) {
                if(cells.contains(c + neighbors[i].offset)) bits |= 1;
                bits <<= 1;
            }

            // place tile
            assert(bits <= lut.size());
            layer.push(c, lut[bits]);
        }
    }

private:
    // given bitstring representing whether or not neighbors are present, figures out the tile to place
    // walks through the rules manually to do so
    TileID determine_tile(uint64_t neighbor_bits) {
        assert(neighbors.size() <= sizeof(uint64_t) * 8);
        for(const auto& r : rules) {
            bool valid = true;
            for(int i = 0; i < neighbors.size(); i++) {
                bool present = (neighbor_bits >> i) & 1;
                if(
                    (present  && r.when[i] == NeighborState::Missing) || 
                    (!present && r.when[i] == NeighborState::Present)
                ) {
                    valid = false;
                    break;
                }
            }
            if(valid) return r.tile;
        }
        assert(false);
    }

    std::vector<Neighbor> neighbors;
    std::vector<Rule> rules;

    // neighbor state lookup table
    // mapping bitstring of neighbor state to tile to place
    std::vector<TileID> lut;
};
    
}