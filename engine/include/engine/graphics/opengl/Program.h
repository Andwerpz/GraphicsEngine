#pragma once
#include <cstdint>

namespace engine::graphics::opengl {

// a ComputeProgram represents some code that is explicitly meant to be ran with some 
//   general purpose computation kernel

// a GraphicsProgram is more specialized for geometric primitive rendering
// the graphics pipeline should specify what sort of geometric primitive it can handle 

class GraphicsDevice;

class ComputeProgram {
private:
    friend class GraphicsDevice;

    // opaque handle managed by the underlying graphics API
    uint64_t handle;

    // the device that created this buffer
    GraphicsDevice* device;

    ~ComputeProgram();
};


enum class PrimitiveTopology {
    // every vertex is a point
    PointList,

    // every group of two vertices is an independent line segment
    LineList,

    // every group of three vertices is an independent triangle
    TriangleList,
};

class GraphicsProgram {
public:
    PrimitiveTopology get_primitive_topology() const {
        return primitive_topology;
    }

private:
    friend class GraphicsDevice;

    // opaque handle managed by the underlying graphics API
    uint64_t handle;

    // the device that created this buffer
    GraphicsDevice* device;

    // the sort of primitives that this graphics pipeline expects
    PrimitiveTopology primitive_topology;

    ~GraphicsProgram();
};

}