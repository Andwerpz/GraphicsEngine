#pragma once
#include <engine/tile/TileID.h>

namespace engine::tile {

// a TilePlacer should know how to place specific tiles according to some rules onto a grid

class Tileset;

class TilePlacer {
    friend class Tileset;

public:
    virtual ~TilePlacer() = default;
    NON_COPYABLE_NON_MOVABLE(TilePlacer);

    const std::string& get_name() {
        return name;
    }

protected:
    TilePlacer(
        std::string name
    ) : name(name) {}

private:
    std::string name;
};
    
}