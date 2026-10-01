#include <engine/graphics/Sampler.h>

namespace engine::graphics {

Sampler::Sampler(Sampler&& other) {
    min_filter = other.min_filter;
    mag_filter = other.mag_filter;
    mipmap_filter = other.mipmap_filter;
    address_u = other.address_u;
    address_v = other.address_v;
    address_w = other.address_w;
}

Sampler::Sampler(
    SamplerFilter _min_filter,
    SamplerFilter _mag_filter,
    SamplerMipmapFilter _mipmap_filter,
    SamplerAddressMode _address_u,
    SamplerAddressMode _address_v,
    SamplerAddressMode _address_w
) {
    min_filter = _min_filter;
    mag_filter = _mag_filter;
    mipmap_filter = _mipmap_filter;
    address_u = _address_u;
    address_v = _address_v;
    address_w = _address_w;
}

Sampler::~Sampler() = default;
    
}