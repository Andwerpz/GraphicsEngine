#include <engine/graphics/Texture.h>
#include <engine/util/error.h>
#include <engine/asset/Image.h>
#include <cassert>

namespace engine::graphics {

Texture::Texture(Texture&& other) {
    texture_id = other.texture_id;
    type = other.type;
    format = other.format;
    width = other.width;
    height = other.height;
    depth = other.depth;
    mip_levels = other.mip_levels;

    other.texture_id = 0;
}

Texture::Texture(
    TextureDimension type, 
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
    texture_id = next_texture_id.fetch_add(1, std::memory_order_relaxed);
}

Texture::~Texture() = default;

uint64_t Texture::get_texel_size() const {
    switch(format) {
    case TextureFormat::RGBA32F:
        return 4 * 4;
    case TextureFormat::Depth32F:
        return 4;
    default:
        assert(false);
    }
}

uint64_t Texture::get_size(uint32_t mip_level) const {
    assert(mip_level < mip_levels);

    // determine size of each texel
    uint64_t tx_size = get_texel_size();

    // determine number of texels
    uint32_t mip_width  = std::max(1u, width  >> mip_level);
    uint32_t mip_height = std::max(1u, height >> mip_level);
    uint32_t mip_depth  = std::max(1u, depth  >> mip_level);
    uint64_t nr_texels = 
        (uint64_t) mip_width * 
        (uint64_t) mip_height * 
        (uint64_t) mip_depth
    ;
    
    return tx_size * nr_texels;
}

void Texture::upload_image(
    engine::asset::Image& image,
    uint32_t mip_level,
    uint32_t cube_face
) {
    // texture must be 2D or cubemap to accept image upload
    if(get_dimension() != TextureDimension::D2 && get_dimension() != TextureDimension::Cube) {
        engine::util::throw_error("texture must be 2D or cubemap to upload data from an image");
    }

    // texture dimensions must match image dimensions
    if(get_width(mip_level) != image.get_width() || get_height(mip_level) != image.get_height()) {
        engine::util::throw_error("image and texture dimensions mismatch");
    }

    // convert image data into proper format
    assert(image.get_format() == engine::asset::ImageFormat::RGBA8);
    void* buf;
    switch(get_format()) {
        case TextureFormat::RGBA32F: {
            uint64_t pixel_count = (uint64_t) image.get_width() * (uint64_t) image.get_height();
            uint64_t buf_size = sizeof(float) * 4 * pixel_count;
            assert(buf_size == get_size(mip_level));
            buf = malloc(buf_size);

            const uint8_t* src = (const uint8_t*) image.get_data();
            float* dst = (float*) buf;
            bool premultiply = image.get_alpha_mode() != engine::asset::ImageAlphaMode::Premultiplied;
            for(uint64_t i = 0; i < pixel_count; i++) {
                float a = src[i * 4 + 3] / 255.0f;
                for(int c = 0; c < 3; c++) {
                    float v = src[i * 4 + c] / 255.0f;
                    dst[i * 4 + c] = premultiply ? v * a : v;
                }
                dst[i * 4 + 3] = a;
            }
            break;
        }
        default:
            engine::util::throw_error("texture has format incompatible with image upload");
    }

    // upload data
    upload_data(buf, mip_level, cube_face);
    free(buf);
}

}