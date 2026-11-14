#pragma once
#include <map>
#include <engine/asset/Image.h>
#include <engine/util/error.h>
#include <engine/util/macro.h>

namespace engine::tile {

// holds a set of channels meant to be textures for tile rendering
// a 'channel' is just an image referred to by a channel name
// all atlases in one Tileset should have the exact same set of channel names
// all images in an atlas should have the same dimensions

class Tileset;

class TileAtlas {
    using Image = engine::asset::Image;
    friend class Tileset;

public:
    const std::string& get_name() const {
        return name;
    }
    const Image& get_channel(std::string name) const {
        if(!channels.contains(name)) {
            engine::util::throw_error("atlas does not have channel with name : " + name);
        }
        return channels.at(name);
    }

private:
    TileAtlas(
        std::string name,
        std::map<std::string, Image>&& channels
    ) : name(name), channels(std::move(channels)) {}
    NON_COPYABLE_NON_MOVABLE(TileAtlas);

    std::string name;
    std::map<std::string, Image> channels;
};

}