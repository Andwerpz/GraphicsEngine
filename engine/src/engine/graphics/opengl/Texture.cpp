#include <engine/graphics/opengl/Texture.h>
#include <engine/graphics/opengl/GraphicsDevice.h>
#include <cassert>

namespace engine::graphics::opengl {

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

Texture::~Texture() {
    device->delete_texture(handle);
}

}

