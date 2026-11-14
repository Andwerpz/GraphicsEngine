#include <engine/tile/Tileset.h>
#include <set>
#include <json.hpp>
using json = nlohmann::ordered_json;

#include <engine/tile/placer/RandomPlacer.h>
#include <engine/tile/placer/StampPlacer.h>
#include <engine/tile/placer/AutotilePlacer.h>
#include <engine/tile/placer/WFCPlacer.h>

namespace engine::tile {

/*
Structure of tileset json:

{

# tileset metadata
"name" : "my_tileset",
"tile_size" : 16,

# set of channel names
# channel names must be unique
# all atlases must have an image for each channel
"channels" : [
    "albedo",
    "normal",
],

# list of all atlases
# atlas names should all be unique
"atlases" : [
    {
        "name" : "atlas1",
        "channels" : {
            "albedo" : "res/albedo.png",
            "normal" : "res/normal.png",
        },
    },
],

# list of all tiles
# all tile names should be unique
"tiles" : [
    {
        "name" : "floor_tile_plain"
        "atlas" : "atlas1",
        "pos" : {
            "x" : 0,
            "y" : 0,
        },
    },
    {
        "name" : "floor_tile_mini",
        "atlas" : "atlas1",
        "pos" : {
            "x" : 16,
            "y" : 0,
        },
    },
],

# list of all placers
# placer names should all be unique
"placers" : [
    # look in TilePlacer.cpp for json format of placers
],

}

*/

namespace {

// { "x" : 0, "y" : 0 }
glm::ivec2 parse_ivec2(const json& v) {
    return { v.at("x").get<int>(), v.at("y").get<int>() };
}

template<class F>
void try_with_error(const std::string& where, F&& f) {
    try { 
        f(); 
    }
    catch(const json::exception& e) { 
        engine::util::throw_error("error loading tileset : " + where + " : " + e.what());
    }
}

struct IVec2Less {
    bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
        if (a.x != b.x) return a.x < b.x;
        return a.y < b.y;
    }
};

TileID find_tile(
    const std::string& name,
    const std::map<std::string, TileID>& tiles_by_name
) {
    auto it = tiles_by_name.find(name);
    if(it == tiles_by_name.end()) {
        engine::util::throw_error("no tile exists with name : " + name);
    }
    return it->second;
}

/*
# STAMP : place a list of tiles in predetermined offsets from an anchor cell
# all offsets should be unique
{
    "name" : "door",
    "type" : "stamp",
    "tiles" : [
        {
            "tile" : "door_lower",
            "offset" : {
                "x" : 0,
                "y" : 0,
            },
        },
        {
            "tile" : "door_upper",
            "offset" : {
                "x" : 0,
                "y" : -1,
            },
        },
    ],
}
*/
std::unique_ptr<TilePlacer> parse_stamp(
    const std::string& name,
    const json& j,
    const std::map<std::string, TileID>& tiles_by_name
) {
    std::vector<StampPlacer::Entry> entries;
    std::set<glm::ivec2, IVec2Less> seen_offsets;

    const json& tiles = j.at("tiles");
    for(int i = 0; i < tiles.size(); i++) {
        const json& t = tiles[i];
        glm::ivec2 offset = parse_ivec2(t.at("offset"));
        if(seen_offsets.contains(offset)) {
            engine::util::throw_error(
                "stamp offsets must be unique, found duplicate : (" +
                    std::to_string(offset.x) + ", " + 
                    std::to_string(offset.y) + 
                ")"
            );
        }
        seen_offsets.emplace(offset);

        entries.push_back({
            .tile = find_tile(t.at("tile").get<std::string>(), tiles_by_name),
            .offset = offset
        });
    }

    if(entries.empty()) {
        engine::util::throw_error("stamp must contain at least one tile");
    }
    return std::make_unique<StampPlacer>(name, std::move(entries));
}

/*
# RANDOM : pick from a weighted list for each cell
# TODO support distributing rectangular stamps that are larger than a single cell
# TODO we should try to make weight represent the amount of area that a cell will take
#   up in the final distribution. Can perhaps force this by making tiles that 
#   have failed to place more likely in the future. 
{
    "name" : "floor",
    "type" : "random",
    "tiles" : [
        {
            "tile" : "floor_normal",
            "weight" : 4.0,
        },
        {
            "tile" : "floor_cracked",
            "weight" : 1.0,
        },
    ],
}
*/
std::unique_ptr<TilePlacer> parse_random(
    const std::string& name,
    const json& j,
    const std::map<std::string, TileID>& tiles_by_name
) {
    std::vector<RandomPlacer::Entry> entries;

    const json& tiles = j.at("tiles");
    for(int i = 0; i < tiles.size(); i++) {
        const json& t = tiles[i];
        float weight = t.at("weight").get<float>();
        if(weight <= 0.0) {
            engine::util::throw_error("random weights must be positive");
        }
        entries.push_back({
            .tile = find_tile(t.at("tile").get<std::string>(), tiles_by_name),
            .weight = weight
        });
    }

    if(entries.empty()) {
        engine::util::throw_error("random placer must contain at least one tile");
    }
    return std::make_unique<RandomPlacer>(name, std::move(entries));
}

/*
# AUTOTILE : select a tile for each cell by looking at what neighbors are occupied
# no two neighbors can have the same name *or* offset
# to choose each cell we have a list of rules. The first rule that matches wins
{
    "name" : "ceiling_trim",
    "type" : "autotile",
    "neighbors" : [
        { "name" : "n",  "offset" : { "x" :  0, "y" : -1 } },
        { "name" : "e",  "offset" : { "x" :  1, "y" :  0 } },
        { "name" : "s",  "offset" : { "x" :  0, "y" :  1 } },
        { "name" : "w",  "offset" : { "x" : -1, "y" :  0 } },
        { "name" : "ne", "offset" : { "x" :  1, "y" : -1 } },
        # ... and so on ...
    ],
    "rules" : [
        {
            "when" : { "n" : "missing", "w" : "missing", "e" : "present", "s" : "present" },
            "tile" : "trim_corner_nw",
        },
        {
            "when" : { "n" : "missing", "e" : "present", "w" : "present" },
            "tile" : "trim_top",
        },
        { 
            "when" : { }, 
            "tile" : "trim_interior",
        },
    ],
}
*/
std::unique_ptr<TilePlacer> parse_autotile(
    const std::string& name,
    const json& j,
    const std::map<std::string, TileID>& tiles_by_name
) {
    // neighbors
    std::vector<AutotilePlacer::Neighbor> neighbors;
    std::map<std::string, int> neighbor_names;
    std::set<glm::ivec2, IVec2Less> neighbor_offsets;

    const json& _neighbors = j.at("neighbors");
    for(int i = 0; i < _neighbors.size(); i++) {
        const json& n = _neighbors[i];
        std::string _name = n.at("name").get<std::string>();
        glm::ivec2 _offset = parse_ivec2(n.at("offset"));

        if(neighbor_names.contains(_name)) {
            engine::util::throw_error("neighbor names must be unique, found duplicate : " + _name);
        }
        if(neighbor_offsets.contains(_offset)) {
            engine::util::throw_error("neighbor offsets must be unique");
        }
        neighbor_names.emplace(_name, i);
        neighbor_offsets.emplace(_offset);

        neighbors.push_back({
            .name = _name,
            .offset = _offset
        });
    }

    // rules
    std::vector<AutotilePlacer::Rule> rules;

    const json& _rules = j.at("rules");
    for(int i = 0; i < _rules.size(); i++) {
        const json& r = _rules[i];

        AutotilePlacer::Rule rule;
        rule.when.assign(neighbors.size(), AutotilePlacer::NeighborState::Any);
        for(auto& [n_name, _state] : r.at("when").items()) {
            auto it = neighbor_names.find(n_name);
            if(it == neighbor_names.end()) {
                engine::util::throw_error("rule refers to unknown neighbor : " + n_name);
            }
            const std::string state = _state.get<std::string>();
            if(state == "present") rule.when[it->second] = AutotilePlacer::NeighborState::Present;
            else if(state == "missing") rule.when[it->second] = AutotilePlacer::NeighborState::Missing;
            else {
                engine::util::throw_error("neighbor state must be \"present\" or \"missing\", found : " + state);
            }
        }
        rule.tile = find_tile(r.at("tile").get<std::string>(), tiles_by_name);
        rules.push_back(std::move(rule));
    }

    return std::make_unique<AutotilePlacer>(
        name, 
        std::move(neighbors), 
        std::move(rules)
    );
}

std::unique_ptr<TilePlacer> parse_placer(
    const json& j,
    const std::map<std::string, TileID>& tiles_by_name
) {
    std::string name = j.at("name").get<std::string>();
    const std::string type = j.at("type").get<std::string>();

    if(type == "stamp") {
        return parse_stamp(name, j, tiles_by_name);
    }
    if(type == "random") {
        return parse_random(name, j, tiles_by_name);
    }
    if(type == "autotile") {
        return parse_autotile(name, j, tiles_by_name);
    }
    if(type == "wfc") {
        assert(false);
    }
    engine::util::throw_error("unknown placer type : " + type);
}

} // namespace

class TilesetBuilder {
public:
    TilesetBuilder() {}
    NON_COPYABLE_NON_MOVABLE(TilesetBuilder);

    void set_name(std::string _name) {
        name = _name;
    }
    void set_tile_size(uint64_t _tile_size) {
        tile_size = _tile_size;
    }
    void add_channel(std::string _channel_name) {
        if(channels.contains(_channel_name)) {
            engine::util::throw_error("channel names must be unique, found duplicate : " + _channel_name);
        }
        channels.insert(_channel_name);
    }

    void add_atlas(std::unique_ptr<TileAtlas>&& atlas) {
        if(atlases.contains(atlas->get_name())) {
            engine::util::throw_error("atlas names must be unique, found duplicate : " + atlas->get_name());
        }
        std::string atlas_name = atlas->get_name();
        atlases.emplace(atlas_name, std::move(atlas));
    }
    void add_tile(Tile&& tile) {
        if(tile_names.contains(tile.name)) {
            engine::util::throw_error("tile names should be unique, found duplicate : " + tile.name);
        }
        std::string tile_name = tile.name;
        TileID id = tiles.size();
        tile.id = id;
        tiles.push_back(std::move(tile));
        tiles_by_name.emplace(tile_name, id);
        tile_names.insert(tile_name);
    }
    void add_placer(std::unique_ptr<TilePlacer>&& placer) {
        if(placers.contains(placer->get_name())) {
            engine::util::throw_error("placer names should be unique, found duplicate : " + placer->get_name());
        }
        std::string placer_name = placer->get_name();
        placers.emplace(placer_name, std::move(placer));
    }

    Tileset create(std::string_view addr) {
        return Tileset( 
            addr,
            name, tile_size,
            std::move(atlases),
            std::move(tiles),
            std::move(placers),
            std::move(tiles_by_name)
        );
    }

    const TileAtlas& get_atlas(std::string name) {
        if(!atlases.contains(name)) {
            engine::util::throw_error("no atlas exists with name : " + name);
        }
        return *atlases.at(name);
    }

    const std::map<std::string, TileID>& get_tiles_by_name() {
        return tiles_by_name;
    }

private:
    std::string name;
    uint64_t tile_size;
    
    std::set<std::string> channels;
    std::set<std::string> tile_names;

    std::map<std::string, std::unique_ptr<TileAtlas>> atlases;
    std::vector<Tile> tiles;
    std::map<std::string, std::unique_ptr<TilePlacer>> placers;

    std::map<std::string, TileID> tiles_by_name;
};

Tileset Tileset::load(std::string_view addr) {
    const std::string file(addr);

    // read + parse
    std::vector<std::byte> bytes = engine::asset::load_asset(addr);
    json j;
    try {
        const char* begin = reinterpret_cast<const char*>(bytes.data());
        j = json::parse(
            begin, begin + bytes.size(),
            /* callback = */ nullptr,
            /* allow_exceptions = */ true,
            /* ignore_comments = */ true
        );
    } 
    catch (const json::parse_error& e) {
        engine::util::throw_error("failed to parse tileset : " + file + " : " + e.what());
    }

    TilesetBuilder builder;

    // tileset metadata
    try_with_error(file, [&] {
        std::string name = j.at("name").get<std::string>();
        int tile_size = j.at("tile_size").get<int>();
        if(tile_size <= 0) {
            engine::util::throw_error("tile_size must be positive");
        }
        builder.set_name(name);
        builder.set_tile_size(tile_size);
    });

    // channel names
    try_with_error(file, [&] {
        std::vector<std::string> names = j.at("channels").get<std::vector<std::string>>();
        for(const auto& name : names) {
            builder.add_channel(name);
        }
    });

    // atlases
    try_with_error(file, [&] {
        const json& atlases = j.at("atlases");
        for(int i = 0; i < atlases.size(); i++) {
            const json& atlas = atlases[i];
            std::string name = atlas.at("name").get<std::string>();

            std::map<std::string, engine::asset::Image> channels;
            const json& _channels = atlas.at("channels");
            for(auto& [key, path] : _channels.items()) {
                channels.emplace(
                    key, 
                    engine::asset::Image::load(path.get<std::string>())
                );
            }

            builder.add_atlas(
                std::unique_ptr<TileAtlas>(new TileAtlas(name, std::move(channels)))
            );
        }
    });

    // tiles
    try_with_error(file, [&] {
        const json& tiles = j.at("tiles");
        for(int i = 0; i < tiles.size(); i++) {
            const json& tile = tiles[i];
            builder.add_tile(
                Tile {
                    .name = tile.at("name").get<std::string>(),
                    .atlas = builder.get_atlas(tile.at("atlas").get<std::string>()),
                    .pos = parse_ivec2(tile.at("pos"))
                }
            );
        }
    });

    // placers
    try_with_error(file, [&] {
        const json& placers = j.at("placers");
        for(int i = 0; i < placers.size(); i++) {
            builder.add_placer(parse_placer(placers[i], builder.get_tiles_by_name()));
        }
    });

    return builder.create(addr);
}

void Tileset::export_tileset(std::string_view addr) {
    // TODO
    assert(false);
}

}



