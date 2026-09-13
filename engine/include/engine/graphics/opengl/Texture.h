#pragma once
#include <cstdint>
#include <math.h>

namespace engine::graphics::opengl {

// a Texture represents some memory on the GPU 
// the structure of the memory is determined by its type and format

// a TextureView represents one way of sampling a Texture 
// TextureViews can be used in graphics pipelines 

class GraphicsDevice;

enum class TextureType {
    D1,
    D2,
    D3,
    Cube,
};

enum class TextureFormat {
    RGBA32F,
    RGBA16F,
};

class Texture {
public:
    TextureType get_type() const {
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

    // returns the size in bytes of the specified mip level
    // for a cubemap, this is the size of one face at the specified mip level
    uint64_t get_size(uint32_t mip_level) const;

private:
    friend class GraphicsDevice;

    // opaque handle managed by the underlying graphics API
    uint64_t handle;

    // the device that created this buffer
    GraphicsDevice* device;

    // what sort of texture is this
    TextureType type;

    // specifies how the texture is stored in GPU memory
    TextureFormat format;

    // (x, y, z) texture dimensions
    uint32_t width;
    uint32_t height;
    uint32_t depth;

    // number of mipmap levels 
    uint32_t mip_levels;

    ~Texture();
};

class TextureView {
    
};

}