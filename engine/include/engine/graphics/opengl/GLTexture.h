#pragma once
#include <cstdint>
#include <math.h>
#include <glad/glad.h>

#include "../Texture.h"

namespace engine::graphics::opengl {

class GLGraphicsDevice;

class GLTexture : public engine::graphics::Texture {
    friend class GLGraphicsDevice;
    using TextureDimension = engine::graphics::TextureDimension;
    using TextureFormat = engine::graphics::TextureFormat;

public:
    GLTexture(GLTexture&&);
    ~GLTexture(); 

    GLuint get_handle() {
        return handle;
    }

    void upload_data(
        const void* data,
        uint32_t mip_level,
        uint32_t cube_face
    ) override;

    void retrieve_data(
        void* data,
        uint32_t mip_level,
        uint32_t cube_face
    ) override;

    void generate_mipmaps() override;

private:
    GLTexture(
        TextureDimension type, 
        TextureFormat format, 
        uint32_t width, 
        uint32_t height, 
        uint32_t depth, 
        uint32_t mip_levels,
        GLGraphicsDevice& device, 
        GLuint handle
    );

    GLTexture() = delete;
    GLTexture(const GLTexture&) = delete;
    GLTexture& operator=(const GLTexture&) = delete;
    GLTexture& operator=(GLTexture&&) = delete;

    // the device that created this texture
    GLGraphicsDevice& device;

    // OpenGL resource handle
    GLuint handle;
};

}