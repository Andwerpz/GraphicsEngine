#pragma once

namespace engine::graphics {

// a Sampler determines how a Texture is sampled

// how to sample between texels
enum class SamplerFilter {
    Nearest,    // clamp to nearest texel
    Linear,     // linearly interpolate between nearby texels
};

// how to sample between two mipmaps
enum class SamplerMipmapFilter {
    None,       // don't use mipmaps
    Nearest,    // clamp to nearest mipmap level
    Linear,     // linearly interpolate between nearest mipmap levels
};

// how sampling behaves when sampling outside of the texture
enum class SamplerAddressMode {
    Repeat,                 // mod coordinates by 1.0
    MirroredRepeat,         // mirror residuals of odd coordinates
    ClampToEdge,            // clamp to edge of texture
    ClampToBorder,          // clamp to some predefined border color
};

class Sampler {
public:
    Sampler(Sampler&& other);
    virtual ~Sampler() = 0;

protected:
    Sampler(
        SamplerFilter min_filter,
        SamplerFilter mag_filter,
        SamplerMipmapFilter mipmap_filter,
        SamplerAddressMode address_u,
        SamplerAddressMode address_v,
        SamplerAddressMode address_w
    );

    SamplerFilter min_filter;
    SamplerFilter mag_filter;
    SamplerMipmapFilter mipmap_filter;

    SamplerAddressMode address_u;
    SamplerAddressMode address_v;
    SamplerAddressMode address_w;

public:
    SamplerFilter get_min_filter() {
        return min_filter;
    }

    SamplerFilter get_mag_filter() {
        return mag_filter;
    }

};

}