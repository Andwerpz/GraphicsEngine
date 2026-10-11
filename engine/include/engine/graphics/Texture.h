#pragma once
#include <cstdint>
#include <math.h>
#include <atomic>

namespace engine::asset {
class Image;
}

namespace engine::graphics {

// a Texture represents some memory on the GPU 
// the structure of the memory is determined by its type and format

// specifies the shape of the texture data
enum class TextureDimension {
    D1,
    D2,
    D3,
    Cube,
};

// specifies the logical texel format
enum class TextureFormat {
    RGBA32F,
    Depth32F,
};

class Texture {
public:
    Texture(Texture&& other);
    virtual ~Texture() = 0;

protected:
    Texture(
        TextureDimension type, 
        TextureFormat format, 
        uint32_t width, 
        uint32_t height, 
        uint32_t depth, 
        uint32_t mip_levels
    );

    inline static std::atomic<uint64_t> next_texture_id{1};

    // unique ID assigned to every texture, texture_id = 0 is invalid
    uint64_t texture_id;

    // what sort of texture is this
    TextureDimension type;

    // specifies how the texture is stored in GPU memory
    TextureFormat format;

    // (x, y, z) texture dimensions
    uint32_t width;
    uint32_t height;
    uint32_t depth;

    // number of mipmap levels 
    uint32_t mip_levels;

public:
    uint64_t get_texture_id() const {
        return texture_id;
    }

    TextureDimension get_dimension() const {
        return type;
    }

    TextureFormat get_format() const {
        return format;
    }

    // get the dimensions of a specified mip level
    uint32_t get_width(uint32_t mip_level = 0) const {
        return std::max(1u, width >> mip_level);
    }
    uint32_t get_height(uint32_t mip_level = 0) const {
        return std::max(1u, height >> mip_level);
    }
    uint32_t get_depth(uint32_t mip_level = 0) const {
        return std::max(1u, depth >> mip_level);
    }

    uint32_t get_mip_levels() const {
        return mip_levels;
    }

    // returns the size in bytes of a single texel
    uint64_t get_texel_size() const;

    // returns the size in bytes of the specified mip level
    // for a cubemap, this is the size of one face at the specified mip level
    uint64_t get_size(uint32_t mip_level) const;

    // upload data to replace whatever was already in the texture
    // data needs to be formatted according to this textures TextureFormat
    //   RGBA32F expects each texel to be 4 floats
    //   Depth32F expects each texel to be 1 float
    // data needs to contain enough data to replace the entire specified mip level
    // you can figure out the size of data needed by using get_size(mip_level)
    virtual void upload_data(
        const void* data,
        uint32_t mip_level = 0,
        uint32_t cube_face = 0
    ) = 0;

    // retrieves data from the texture uploaded on the GPU
    // data needs to point to some underlying memory of size get_size(mip_level)
    // overwrites memory pointed to by data to be texture data 
    virtual void retrieve_data(
        void* data,
        uint32_t mip_level = 0,
        uint32_t cube_face = 0
    ) = 0;

    // uploads data directly from the provided image
    // this texture must be 2D or a Cubemap
    // the dimensions of the layer must match the image dimensions
    void upload_image(
        engine::asset::Image& image,
        uint32_t mip_level = 0,
        uint32_t cube_face = 0
    );

    // generates mipmaps using mip level 0 as the base
    // replaces all other mip layers
    virtual void generate_mipmaps() = 0;
};

}