#include <engine/graphics/opengl/GLTexture.h>
#include <engine/graphics/opengl/GLGraphicsDevice.h>
#include <cassert>
#include <engine/utils/error.h>

namespace engine::graphics::opengl {

GLTexture::GLTexture(GLTexture&& other) : 
    engine::graphics::Texture(std::move(other)),
    device(other.device),
    handle(other.handle)
{
    other.handle = 0;
}

GLTexture::~GLTexture() {
    if(handle != 0) {
        device.gl_delete_texture(handle);
    }
}

GLTexture::GLTexture(
    TextureType type, 
    TextureFormat format, 
    uint32_t width, 
    uint32_t height, 
    uint32_t depth, 
    uint32_t mip_levels,
    GLGraphicsDevice& device, 
    GLuint handle
) : Texture(type, format, width, height, depth, mip_levels),
    device(device),
    handle(handle) 
{
    // do nothing
}

void GLTexture::upload_data(
    const void* data,
    uint32_t mip_level,
    uint32_t cube_face
) {
    if(data == nullptr) {
        engine::utils::throw_error("data cannot be nullptr");
    }
    if(mip_level >= get_mip_levels()) {
        engine::utils::throw_error("mip_level out of range");
    }
    if(get_type() == TextureType::Cube) {
        if(cube_face > 6) {
            engine::utils::throw_error("cube_face should be < 6");
        }
    }
    else {
        if(cube_face > 0) {
            engine::utils::throw_error("cube_face should be 0 for non-cubemap texture");
        }
    }

    GLuint gl_handle = handle;
    uint64_t data_size = get_size(mip_level);
    GLenum gl_format = GLGraphicsDevice::gl_format(get_format());
    GLenum gl_type = GLGraphicsDevice::gl_type(get_format());
    switch(get_type()) {
        case TextureType::D1:
            glTextureSubImage1D(
                gl_handle,
                mip_level,
                0,
                get_width(mip_level),
                gl_format,
                gl_type,
                data
            );
            break;
        case TextureType::D2:
            glTextureSubImage2D(
                gl_handle,
                mip_level,
                0, 0,
                get_width(mip_level),
                get_height(mip_level),
                gl_format,
                gl_type,
                data
            );
            break;
        case TextureType::D3:
            glTextureSubImage3D(
                gl_handle,
                mip_level,
                0, 0, 0,
                get_width(mip_level),
                get_height(mip_level),
                get_depth(mip_level),
                gl_format,
                gl_type,
                data
            );
            break;
        case TextureType::Cube:
            assert(cube_face < 6);
            glBindTexture(GL_TEXTURE_CUBE_MAP, gl_handle);
            glTexSubImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + cube_face,
                mip_level,
                0, 0,
                get_width(mip_level),
                get_height(mip_level),
                gl_format,
                gl_type,
                data
            );
            break;
        default:
            assert(false);
    }
}

void GLTexture::generate_mipmaps() {
    glGenerateTextureMipmap(handle);
}

}

