#pragma once
#include <cstdint>
#include <glad/glad.h>

namespace engine::graphics::opengl {

// a GLComputeProgram represents some code that is explicitly meant to be ran with some 
//   general purpose computation kernel

// a GLGraphicsProgram is more specialized for geometric primitive rendering
// the graphics pipeline should specify what sort of geometric primitive it can handle 

class OpenGLGraphicsDevice;

class GLComputeProgram {
public:
    GLComputeProgram(GLComputeProgram&&);
    ~GLComputeProgram();

private:
    friend class OpenGLGraphicsDevice;

    GLComputeProgram(OpenGLGraphicsDevice& device, GLuint handle) :
        device(device), handle(handle) {}

    GLComputeProgram() = delete;
    GLComputeProgram(const GLComputeProgram&) = delete;
    GLComputeProgram& operator=(const GLComputeProgram&) = delete;
    GLComputeProgram& operator=(GLComputeProgram&&) = delete;

    // the device that created this program
    OpenGLGraphicsDevice& device;

    // OpenGL resource handle
    GLuint handle;
};


enum class PrimitiveTopology {
    // every vertex is a point
    PointList,

    // every group of two vertices is an independent line segment
    LineList,

    // every group of three vertices is an independent triangle
    TriangleList,
};

class GLGraphicsProgram {
public:
    GLGraphicsProgram(GLGraphicsProgram&&);
    ~GLGraphicsProgram();

    PrimitiveTopology get_primitive_topology() const {
        return primitive_topology;
    }

private:
    friend class OpenGLGraphicsDevice;

    GLGraphicsProgram(OpenGLGraphicsDevice& device, GLuint handle, PrimitiveTopology primitive_topology) :
        device(device), handle(handle), primitive_topology(primitive_topology) {}

    GLGraphicsProgram() = delete;
    GLGraphicsProgram(const GLGraphicsProgram&) = delete;
    GLGraphicsProgram& operator=(const GLGraphicsProgram&) = delete;
    GLGraphicsProgram& operator=(GLGraphicsProgram&&) = delete;

    // the device that created this program
    OpenGLGraphicsDevice& device;

    // OpenGL resource handle
    GLuint handle;

    // the sort of primitives that this graphics pipeline expects
    PrimitiveTopology primitive_topology;
};

}