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
        device.gl_delete_texture(*this);
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
    void* data,
    uint32_t mip_level,
    uint32_t cube_face
) {
    if(data == nullptr) {
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

    GLenum gl_format = GLGraphicsDevice::gl_format(get_format());
    GLenum gl_type = GLGraphicsDevice::gl_type(get_format());
    GLint x = 0;
    GLint y = 0;
    GLint z = 0;
    GLsizei width = static_cast<GLsizei>(get_width(mip_level));
    GLsizei height = 1;
    GLsizei depth = 1;
    switch (get_dimension()) {
        case TextureDimension::D1:
            break;
        case TextureDimension::D2:
            height = static_cast<GLsizei>(get_height(mip_level));
            break;
        case TextureDimension::D3:
            height = static_cast<GLsizei>(get_height(mip_level));
            depth = static_cast<GLsizei>(get_depth(mip_level));
            break;
        case TextureDimension::Cube:
            height = static_cast<GLsizei>(get_height(mip_level));
            z = static_cast<GLint>(cube_face);
            break;
        default:
            assert(false);
            return;
    }

    glGetTextureSubImage(
        handle,
        static_cast<GLint>(mip_level),
        x, y, z,
        width, height, depth,
        gl_format,
        gl_type,
        static_cast<GLsizei>(get_size(mip_level)),
        data
    );
}

void GLTexture::generate_mipmaps() {
    glGenerateTextureMipmap(handle);
}

}

