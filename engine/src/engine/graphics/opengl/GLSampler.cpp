#include <engine/graphics/opengl/GLSampler.h>
#include <engine/graphics/opengl/GLGraphicsDevice.h>
#include <cassert>
#include <engine/utils/error.h>

namespace engine::graphics::opengl {

GLSampler::GLSampler(GLSampler&& other) :
    engine::graphics::Sampler(std::move(other)),
    device(other.device),
    handle(other.handle)
{
    other.handle = 0;
}

GLSampler::~GLSampler() {
    if(handle != 0) {
        device.gl_delete_sampler(*this);
    }
}

GLSampler::GLSampler(
    SamplerFilter min_filter,
    SamplerFilter mag_filter,
    SamplerMipmapFilter mipmap_filter,
    SamplerAddressMode address_u,
    SamplerAddressMode address_v,
    SamplerAddressMode address_w,
    GLGraphicsDevice& device,
    GLuint handle
) : engine::graphics::Sampler(min_filter, mag_filter, mipmap_filter, address_u, address_v, address_w),
    device(device),
    handle(handle)
{
    // do nothing
}

}