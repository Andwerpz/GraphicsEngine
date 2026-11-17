#pragma once
#include <cstdint>
#include <atomic>
#include <vector>
#include <string>
#include <optional>
#include <string_view>
#include <engine/util/error.h>
#include <engine/util/macro.h>

namespace engine::asset {

using AssetID = uint64_t;

class Asset {
public:
    Asset();
    Asset(std::string_view addr);
    virtual ~Asset() = 0;

    Asset(const Asset& other);
    Asset& operator=(const Asset& other) = delete;
    Asset(Asset&& other);
    Asset& operator=(Asset&& other) = delete;

    AssetID get_asset_id() const {
        return asset_id;
    }

    bool has_address() const {
        return address.has_value();
    }
    const std::string& get_address() const {
        if(!address.has_value()) {
            engine::util::throw_error("Asset does not have address");
        }
        return address.value();
    }

private:
    inline static std::atomic<AssetID> next_asset_id{1};

    // unique ID assigned to every asset, asset_id = 0 is invalid
    // each asset having a unique id + being immutable after construction 
    // makes it so that any downstream users of an asset can easily cache 
    // results related to processing an asset by asset_id
    AssetID asset_id;

    // address used to load this asset
    std::optional<std::string> address;
};

// disambiguates an asset address 
// if the address starts with "engine://", tries to load from embedded resources
// otherwise interprets address as filepath and loads from the filesystem
std::vector<std::byte> load_asset(std::string_view addr);

}