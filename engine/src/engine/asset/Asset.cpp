#include <engine/asset/Asset.h>
#include <engine/asset/EmbeddedResource.h>
#include <engine/util/error.h>
#include <engine/util/file.h>

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

constexpr std::string_view engine_prefix = "engine://";
std::vector<std::byte> load_asset(std::string_view addr) {
    if(addr.starts_with(engine_prefix)) {
        std::string_view rel = addr.substr(engine_prefix.size());
        EmbeddedResource res = EmbeddedResource::load(rel);
        if(!res) {
            engine::util::throw_error("Engine resource not found : " + std::string(addr));
        }
        return { (std::byte*) res.data, (std::byte*) res.data + res.size };
    }
    return engine::util::load_file(fs::path(addr));
}


}