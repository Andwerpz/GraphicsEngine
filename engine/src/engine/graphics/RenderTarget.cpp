#include <engine/graphics/RenderTarget.h>
#include <engine/utils/error.h>
#include <cassert>

namespace engine::graphics {

void RenderTarget::validate() {
    assert(!validated);
    validated = true;
    validate_impl();
}


TextureRenderTarget::TextureRenderTarget(
    uint32_t width,
    uint32_t height
) : width(width),
    height(height)
{
    // do nothing
}

void TextureRenderTarget::bind_texture(uint32_t location, Texture& tex) {
    if(validated) {
        engine::utils::throw_error("cannot bind texture to render target after validating");
    }

    // see if there already is a texture bound at this location
    if(bound_textures.contains(location)) {
        engine::utils::throw_error("texture already bound at location : " + std::to_string(location));
    }

    // make sure dimensions match
    if(tex.get_width() != width || tex.get_height() != height) {
        engine::utils::throw_error("texture dimensions do not match render target dimensions");
    }

    // bind texture
    bound_textures.insert({location, tex});
    bind_texture_impl(location, tex);
}

void TextureRenderTarget::bind_depth_buffer() {
    if(validated) {
        engine::utils::throw_error("cannot bind depth buffer to render target after validating");
    }

    // see if there already is a depth buffer
    if(depth_buffer_bound) {
        engine::utils::throw_error("depth buffer already bound");
    }

    // bind depth buffer
    depth_buffer_bound = true;
    bind_depth_buffer_impl();
}


WindowRenderTarget::WindowRenderTarget(
    Window& window
) : window(window) 
{
    // do nothing
}

}