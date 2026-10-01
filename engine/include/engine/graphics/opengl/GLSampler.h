#pragma once
#include <cstdint>
#include <math.h>
#include <glad/glad.h>

#include "../Sampler.h"

namespace engine::graphics::opengl {

class GLGraphicsDevice;

class GLSampler : public engine::graphics::Sampler {
    friend class GLGraphicsDevice;
    using SamplerFilter = engine::graphics::SamplerFilter;
    using SamplerMipmapFilter = engine::graphics::SamplerMipmapFilter;
    using SamplerAddressMode = engine::graphics::SamplerAddressMode;

public:
    GLSampler(GLSampler&&);
    ~GLSampler();

    GLuint get_handle() {
        return handle;
    }

private:
    GLSampler(
        SamplerFilter min_filter,
        SamplerFilter mag_filter,
        SamplerMipmapFilter mipmap_filter,
        SamplerAddressMode address_u,
        SamplerAddressMode address_v,
        SamplerAddressMode address_w,
        GLGraphicsDevice& device,
        GLuint handle
    );

    GLSampler() = delete;
    GLSampler(const GLSampler&) = delete;
    GLSampler& operator=(const GLSampler&) = delete;
    GLSampler& operator=(GLSampler&&) = delete;

    // the device that created this texture
    GLGraphicsDevice& device;

    // OpenGL resource handle
    GLuint handle;

};

}