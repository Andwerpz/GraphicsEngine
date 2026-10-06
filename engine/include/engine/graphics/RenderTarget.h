#pragma once
#include <cstdint>
#include <map>
#include <functional>

#include "Texture.h"
#include "Window.h"

namespace engine::graphics {

// represents some collection of 2D things that can be rendered to
// a TextureRenderTarget represents a list of textures that can be rendered to simultaneously
// a WindowRenderTarget represents rendering to a Window

class Window;

class RenderTarget {
public:
    // get dimensions
    virtual uint32_t get_width() = 0;
    virtual uint32_t get_height() = 0;

    // returns true if this render target has a depth buffer 
    virtual bool has_depth_buffer() = 0;

    // returns true if this render target has an output at this location, false otherwise
    virtual bool has_output(uint32_t location) = 0;

    // locks this render target 
    // after doing this, you shouldn't be able to mutate resources attached to this render target
    void validate();

    bool is_validated() {
        return validated;
    }

protected:
    virtual void validate_impl() = 0;
    bool validated = false;
};

class TextureRenderTarget : public RenderTarget {
public:
    uint32_t get_width() override {
        return width;
    }
    uint32_t get_height() override {
        return height;
    }
    bool has_depth_buffer() override {
        return depth_buffer_bound;
    }
    bool has_output(uint32_t location) override {
        return bound_textures.contains(location);
    }

    // bind a texture to the given location
    // texture should be 2D and have the same dimensions as the render target
    // you cannot bind multiple textures to the same location
    // cannot do this after validating
    void bind_texture(uint32_t location, Texture& tex);

    // create and bind an internally managed depth buffer
    // can only do this at most once
    // cannot do this after validating
    void bind_depth_buffer();

protected:
    TextureRenderTarget(uint32_t width, uint32_t height);

    // render target dimensions
    uint32_t width;
    uint32_t height;

    // map of location -> texture
    std::map<uint32_t, std::reference_wrapper<Texture>> bound_textures;

    // whether or not we've bound a depth buffer yet
    bool depth_buffer_bound = false;

    virtual void bind_texture_impl(uint32_t location, Texture& tex) = 0;
    virtual void bind_depth_buffer_impl() = 0;
};

class WindowRenderTarget : public RenderTarget {
public:
    uint32_t get_width() override {
        return window.get_width();
    }
    uint32_t get_height() override {
        return window.get_height();
    }

    Window& get_window() {
        return window;
    }

protected:
    WindowRenderTarget(Window& window);

    // underlying window
    Window& window;
};

}