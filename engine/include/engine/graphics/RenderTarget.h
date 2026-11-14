#pragma once
#include <cstdint>
#include <map>
#include <functional>
#include <memory>

#include <engine/util/error.h>
#include <engine/math/Rect.h>

#include "Texture.h"
#include "Window.h"

namespace engine::graphics {

// represents some collection of 2D things that can be rendered to
// a TextureRenderTarget represents a list of textures that can be rendered to simultaneously
// a WindowRenderTarget represents rendering to a Window
// a RenderTargetView represents a subregion of another render target

class Window;

enum class ClearFlags : uint64_t {
    None    = 0,
    Color   = 1 << 0,
    Depth   = 1 << 1,
};

constexpr ClearFlags operator|(ClearFlags lhs, ClearFlags rhs) {
    return static_cast<ClearFlags>(
        static_cast<uint64_t>(lhs) | static_cast<uint64_t>(rhs)
    );
}
constexpr ClearFlags operator&(ClearFlags lhs, ClearFlags rhs) {
    return static_cast<ClearFlags>(
        static_cast<uint64_t>(lhs) & static_cast<uint64_t>(rhs)
    );
}
constexpr ClearFlags operator^(ClearFlags lhs, ClearFlags rhs) {
    return static_cast<ClearFlags>(
        static_cast<uint64_t>(lhs) ^ static_cast<uint64_t>(rhs)
    );
}
constexpr ClearFlags operator~(ClearFlags flags) {
    return static_cast<ClearFlags>(
        ~static_cast<uint64_t>(flags)
    );
}
constexpr ClearFlags& operator|=(ClearFlags& lhs, ClearFlags rhs) {
    return lhs = lhs | rhs;
}
constexpr ClearFlags& operator&=(ClearFlags& lhs, ClearFlags rhs) {
    return lhs = lhs & rhs;
}
constexpr ClearFlags& operator^=(ClearFlags& lhs, ClearFlags rhs) {
    return lhs = lhs ^ rhs;
}

class RenderTargetView;

class RenderTarget {
public:
    using Bounds = engine::math::Rect<2, int>;

    // dimensions and offset of the viewport
    virtual uint32_t get_width() = 0;
    virtual uint32_t get_height() = 0;
    virtual int get_xoff() { return 0; }
    virtual int get_yoff() { return 0; }

    // returns the render target that actually owns the underlying pixels
    // for normal render targets, this should just return itself
    virtual RenderTarget& get_root() { return *this; }

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

    // clears all of the configured attachments within their respective viewports
    // specifically, RenderTargetView::clear() does not clear outside of its view
    // can only be done after validating
    virtual void clear(ClearFlags flags) = 0;

    // creates a RenderTargetView that represents the subrectangle of this target indicated by bounds
    // bounds can be empty, and it can fall outside of and/or be completely disjoint from this target
    // the underlying RenderTarget has to outlive this created view
    virtual std::unique_ptr<RenderTargetView> create_view(Bounds bounds) = 0;

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

class RenderTargetView : public RenderTarget {
public:
    uint32_t get_width() override {
        return bounds.dim.x < 0 ? 0 : bounds.dim.x;
    }
    uint32_t get_height() override {
        return bounds.dim.y < 0 ? 0 : bounds.dim.y;
    }
    int get_xoff() override {
        return bounds.pos.x;
    }
    int get_yoff() override {
        return bounds.pos.y;
    }
    
    RenderTarget& get_root() {
        return root_target;
    }

    bool has_depth_buffer() override {
        return root_target.has_depth_buffer();
    }
    bool has_output(uint32_t location) override {
        return root_target.has_output(location);
    }

protected:
    RenderTargetView(RenderTarget& target, Bounds _bounds) :
        root_target(target.get_root()), bounds(_bounds)
    {   
        // make sure target is validated
        if(!target.is_validated()) {
            engine::util::throw_error("can only make view of validated render target");
        }

        // offset bounds from underlying target
        bounds.pos.x += target.get_xoff();
        bounds.pos.y += target.get_yoff();

        // validate
        validated = true;
    }

    void validate_impl() override {
        engine::util::throw_error("RenderTargetView cannot be validated");
    }

    // root non-view render target
    RenderTarget& root_target;

    // rectangle of the root target that is exposed by this target
    // these bounds should be relative to the root render target
    // these bounds can go outside of the root render target
    // bounds are immutable after construction
    Bounds bounds;
};

}