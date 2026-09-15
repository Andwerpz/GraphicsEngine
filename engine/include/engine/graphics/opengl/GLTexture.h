#pragma once
#include <cstdint>
#include <math.h>
#include <glad/glad.h>

namespace engine::graphics::opengl {

// a GLTexture represents some memory on the GPU 
// the structure of the memory is determined by its type and format

class OpenGLGraphicsDevice;

enum class GLTextureType {
    D1,
    D2,
    D3,
    Cube,
};

enum class GLTextureFormat {
    RGBA32F,
    RGBA16F,
};

class GLTexture {
public:
    GLTexture(GLTexture&&);
    ~GLTexture(); 

    GLTextureType get_type() const {
        return type;
    }

    GLTextureFormat get_format() const {
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
    friend class OpenGLGraphicsDevice;

    GLTexture(OpenGLGraphicsDevice& device, GLuint handle, GLTextureType type, GLTextureFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mip_levels) :
        device(device), handle(handle), type(type), format(format), width(width), height(height), depth(depth), mip_levels(mip_levels) {}

    GLTexture() = delete;
    GLTexture(const GLTexture&) = delete;
    GLTexture& operator=(const GLTexture&) = delete;
    GLTexture& operator=(GLTexture&&) = delete;

    // the device that created this texture
    OpenGLGraphicsDevice& device;

    // OpenGL resource handle
    GLuint handle;

    // what sort of texture is this
    GLTextureType type;

    // specifies how the texture is stored in GPU memory
    GLTextureFormat format;

    // (x, y, z) texture dimensions
    uint32_t width;
    uint32_t height;
    uint32_t depth;

    // number of mipmap levels 
    uint32_t mip_levels;
};

class TextureView {
    
};

}