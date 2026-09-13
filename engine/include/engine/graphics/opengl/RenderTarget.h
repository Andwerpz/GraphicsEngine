#pragma once
#include <cstdint>

namespace engine::graphics::opengl {

// a RenderTarget just represents something that can be rendered to 
// the underlying resource being rendered to is implementation specific 

class GraphicsDevice;

class RenderTarget {
  
private:
    friend class GraphicsDevice;

    // opaque handle managed by the underlying graphics API
    uint64_t handle;

    // the device that created this buffer
    GraphicsDevice* device;

    ~RenderTarget();
};

}