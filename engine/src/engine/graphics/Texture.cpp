#include <engine/graphics/Texture.h>
#include <cassert>

namespace engine::graphics {

Texture::Texture(Texture&& other) {
    type = other.type;
    format = other.format;
    width = other.width;
    height = other.height;
    depth = other.depth;
    mip_levels = other.mip_levels;
}

Texture::Texture(
    TextureType type, 
    TextureFormat format, 
    uint32_t width, 
    uint32_t height, 
    uint32_t depth, 
    uint32_t mip_levels
) : type(type),
    format(format),
    width(width),
    height(height),
    depth(depth),
    mip_levels(mip_levels) {
    // do nothing
}

Texture::~Texture() = default;

uint64_t Texture::get_size(uint32_t mip_level) const {
    assert(mip_level < mip_levels);

    // determine size of each pixel
    uint64_t px_size;
    switch(format) {
    case TextureFormat::RGBA32F:
        px_size = 4 * 4;
        break;
    case TextureFormat::RGBA16F:
        px_size = 2 * 4;
        break;
    default:
        assert(false);
    }

    // determine number of pixels
    uint32_t mip_width  = std::max(1u, width  >> mip_level);
    uint32_t mip_height = std::max(1u, height >> mip_level);
    uint32_t mip_depth  = std::max(1u, depth  >> mip_level);
    uint64_t nr_pixels = 
        (uint64_t) mip_width * 
        (uint64_t) mip_height * 
        (uint64_t) mip_depth
    ;
    
    return px_size * nr_pixels;
}

}