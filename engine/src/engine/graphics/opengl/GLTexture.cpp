#include <engine/graphics/opengl/GLTexture.h>
#include <engine/graphics/opengl/GLGraphicsDevice.h>
#include <engine/utils/error.h>

#include <cstring>
#include <cassert>
#include <memory>

#include <iostream>

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
    TextureDimension type, 
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
    const void* _data,
    uint32_t mip_level,
    uint32_t cube_face
) {
    if(_data == nullptr) {
        engine::utils::throw_error("data cannot be nullptr");
    }
    if(mip_level >= get_mip_levels()) {
        engine::utils::throw_error("mip_level out of range");
    }
    if(get_dimension() == TextureDimension::Cube) {
        if(cube_face >= 6) {
            engine::utils::throw_error("cube_face should be < 6");
        }
    }
    else {
        if(cube_face > 0) {
            engine::utils::throw_error("cube_face should be 0 for non-cubemap texture");
        }
    }

    // OpenGL defines the bottom left of an image as the origin
    // this means that the first texel of the last row is the origin
    // curiously, this flipping is not present in cubemaps
    // see if we need to flip data 
    bool flip_y = false;
    bool flip_z = false;
    switch(get_dimension()) {
        case TextureDimension::D1:
            break;
        case TextureDimension::D2:
            flip_y = true;
            break;
        case TextureDimension::D3:
            flip_y = true;
            flip_z = true;
            break;
        case TextureDimension::Cube:
            break;
        default:
            assert(false);
    }

    // flip data 
    const void* data = _data;
    std::vector<std::byte> flipped_data;
    if(flip_y || flip_z) {
        const uint32_t width = get_width(mip_level);
        const uint32_t height = get_height(mip_level);
        const uint32_t depth = get_depth(mip_level);
        const uint32_t texel_size = get_texel_size();

        const size_t row_size = static_cast<size_t>(width) * texel_size;
        const size_t slice_size = static_cast<size_t>(height) * row_size;
        const size_t data_size = static_cast<size_t>(depth) * slice_size;
        flipped_data.resize(data_size);

        const auto* src = static_cast<const std::byte*>(_data);
        auto* dst = flipped_data.data();
        for(uint32_t z = 0; z < depth; ++z) {
            const uint32_t src_z = flip_z ? (depth - 1 - z) : z;
            for(uint32_t y = 0; y < height; ++y) {
                const uint32_t src_y = flip_y ? (height - 1 - y) : y;
                const size_t src_offset =
                    static_cast<size_t>(src_z) * slice_size +
                    static_cast<size_t>(src_y) * row_size;
                const size_t dst_offset =
                    static_cast<size_t>(z) * slice_size +
                    static_cast<size_t>(y) * row_size;

                std::memcpy(
                    dst + dst_offset,
                    src + src_offset,
                    row_size
                );
            }
        }
        data = flipped_data.data();
    }

    // upload data 
    GLuint gl_handle = handle;
    uint64_t data_size = get_size(mip_level);
    GLenum gl_format = GLGraphicsDevice::gl_format(get_format());
    GLenum gl_type = GLGraphicsDevice::gl_type(get_format());
    switch(get_dimension()) {
        case TextureDimension::D1:
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
        case TextureDimension::D2:
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
        case TextureDimension::D3:
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
        case TextureDimension::Cube:
            assert(cube_face < 6);
            glTextureSubImage3D(
                handle,
                mip_level,
                0, 0, cube_face,
                get_width(mip_level),
                get_height(mip_level),
                1,
                gl_format,
                gl_type,
                data
            );
            break;
        default:
            assert(false);
    }
}

void GLTexture::retrieve_data(
    const void* data,
    uint32_t mip_level,
    uint32_t cube_face
) {
    // TODO
    assert(false);
}

void GLTexture::generate_mipmaps() {
    glGenerateTextureMipmap(handle);
}

}

