#include <engine/asset/Asset.h>

namespace engine::asset {

Asset::Asset() {
    asset_id = next_asset_id.fetch_add(1, std::memory_order_relaxed);
}

Asset::~Asset() = default;

Asset::Asset(Asset&& other) {
    asset_id = other.asset_id;
    other.asset_id = 0;
}

Asset& Asset::operator=(Asset&& other) {
    if(this == &other) return *this;
    asset_id = other.asset_id;
    other.asset_id = 0;
    return *this;
}

}