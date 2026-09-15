#include <engine/graphics/opengl/GLTexture.h>
#include <engine/graphics/opengl/OpenGLGraphicsDevice.h>
#include <cassert>

namespace engine::graphics::opengl {

GLTexture::GLTexture(GLTexture&& other) :
device(other.device), handle(other.handle), type(other.type), format(other.format), 
width(other.width), height(other.height), depth(other.depth), mip_levels(other.mip_levels) {
    other.handle = 0;
}

GLTexture::~GLTexture() {
    if(handle != 0) {
        device.gl_delete_texture(handle);
    }
}

uint64_t GLTexture::get_size(uint32_t mip_level) const {
    assert(mip_level < mip_levels);

    // determine size of each pixel
    uint64_t px_size;
    switch(format) {
    case GLTextureFormat::RGBA32F:
        px_size = 4 * 4;
        break;
    case GLTextureFormat::RGBA16F:
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

