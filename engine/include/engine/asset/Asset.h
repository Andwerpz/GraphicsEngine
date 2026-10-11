#pragma once
#include <cstdint>
#include <atomic>

namespace engine::asset {

class Asset {
public:
    Asset();
    virtual ~Asset() = 0;

    Asset(Asset&& other);
    Asset& operator=(Asset&& other);

    Asset(const Asset&) = delete;
    Asset& operator=(const Asset&) = delete;

    uint64_t get_asset_id() const {
        return asset_id;
    }

private:
    inline static std::atomic<uint64_t> next_asset_id{1};

    // unique ID assigned to every asset, asset_id = 0 is invalid
    // each asset having a unique id + being immutable after construction 
    // makes it so that any downstream users of an asset can easily cache 
    // results related to processing an asset
    uint64_t asset_id;
};

}