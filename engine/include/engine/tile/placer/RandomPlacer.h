#pragma once
#include <vector>
#include <random>
#include <glm/glm.hpp>
#include <engine/tile/placer/TilePlacer.h>
#include <engine/tile/TileGrid.h>
#include <engine/tile/CellSet.h>

namespace engine::tile {

class RandomPlacer : public TilePlacer {
public:
    ~RandomPlacer() override = default;

    struct Entry {
        TileID tile;
        double weight;
    };

    RandomPlacer(
        std::string name,
        std::vector<Entry>&& _entries
    ) : TilePlacer(name),
        entries(std::move(_entries)) 
    {
        // should have at least one entry
        assert(entries.size() != 0);

        // all entries should have positive weight
        for(int i = 0; i < entries.size(); i++) {
            assert(entries[i].weight > 0);
        }

        wpfx.resize(entries.size() + 1);
        wpfx[0] = 0;
        for(int i = 0; i < entries.size(); i++) {
            wpfx[i + 1] = wpfx[i] + entries[i].weight;
        }
    }

    void place_random(TileLayer& layer, const CellSet& cells) const {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dis(0.0, wpfx[entries.size()]);
        for(const glm::ivec2& c : cells) {
            // generate random double
            double x = dis(gen);

            // look for largest index such that x >= wpfx[ind]
            int ind = 0;
            int low = 0, high = entries.size() - 1;
            while(low <= high) {
                int mid = low + (high - low) / 2;
                if(x >= wpfx[mid]) {
                    low = mid + 1;
                    ind = mid;
                }
                else {
                    high = mid - 1;
                }
            }

            // place tile
            assert(0 <= ind && ind < entries.size());
            layer.push(c, entries[ind].tile);
        }
    }

private:
    std::vector<Entry> entries;

    // wpfx[i] = sum of entries[j].weight for 0 <= j < i
    std::vector<double> wpfx;
};

}