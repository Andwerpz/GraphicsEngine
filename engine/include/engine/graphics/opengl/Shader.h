#pragma once
#include <cstdint>

namespace engine::graphics::opengl {

// a Shader represents some compiled code on the GPU 
// the type of a shader determines what sort of pipelines it can participate in, and in what stages

class GraphicsDevice;

enum class ShaderType {
    Vertex,
    Fragment,
    Compute,
};

class Shader {
public:
    ShaderType get_type() const {
        return type;
    }

private:
    friend class GraphicsDevice;

    // opaque handle managed by the underlying graphics API
    uint64_t handle;

    // the device that created this buffer
    GraphicsDevice* device;

    // what sort of shader is this
    // used by pipelines to make sure that you're giving them valid shaders 
    ShaderType type;

    ~Shader();
};

}