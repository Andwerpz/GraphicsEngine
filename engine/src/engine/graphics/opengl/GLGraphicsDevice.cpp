#include <engine/graphics/opengl/GLGraphicsDevice.h>
#include <engine/utils/error.h>

#include <stdexcept>
#include <cassert>
#include <utility>
#include <iostream>
#include <cstring>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shaderc/shaderc.h>

namespace engine::graphics::opengl {

GLGraphicsDevice::GLGraphicsDevice() {
    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "GLFW error " << error << ": " << description << std::endl;
    });

    // initialize glfw
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    if(!glfwInit()) {
        engine::utils::throw_error("Failed to initialize GLFW");
    }
    
    // create opengl graphics device by creating hidden GLFW window
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* glfw_window = glfwCreateWindow(
        1,
        1,
        "",
        nullptr,
        nullptr
    );
    if(glfw_window == nullptr) {
        glfwTerminate();
        engine::utils::throw_error("Failed to create headless GLFW window");
    }

    // set active context to hidden window
    headless_context.emplace(std::move(GLContext(
        *this,
        glfw_window
    )));
    gl_reset_context();

    // load OpenGL function pointers for current thread
    if(gladLoadGL() == 0) {
        glfwDestroyWindow(glfw_window);
        glfwTerminate();
        engine::utils::throw_error("Failed to initialize GLAD");
    }

    std::cout << "OpenGL Renderer : " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Version : " << glGetString(GL_VERSION) << std::endl;
    
    // grab capabilities
    GLint gl_max_uniform_buffer_slots;
    GLint gl_max_storage_buffer_slots;
    GLint gl_max_texture_slots;
    GLint gl_max_sampler_slots;
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &gl_max_uniform_buffer_slots);
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &gl_max_storage_buffer_slots);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &gl_max_texture_slots);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &gl_max_sampler_slots);

    assert(gl_max_uniform_buffer_slots > 0);
    assert(gl_max_storage_buffer_slots > 0);
    assert(gl_max_texture_slots > 0);
    assert(gl_max_texture_slots > 0);

    std::cout << "gl_max_uniform_buffer_slots : " << gl_max_uniform_buffer_slots << "\n";
    std::cout << "gl_max_storage_buffer_slots : " << gl_max_storage_buffer_slots << "\n";
    std::cout << "gl_max_texture_slots : " << gl_max_texture_slots << "\n";
    std::cout << "gl_max_sampler_slots : " << gl_max_sampler_slots << "\n";
    
    bound_uniform_buffers.resize(gl_max_uniform_buffer_slots, std::nullopt);
    bound_storage_buffers.resize(gl_max_storage_buffer_slots, std::nullopt);
    bound_textures.resize(gl_max_texture_slots, std::nullopt);
    bound_samplers.resize(gl_max_sampler_slots, std::nullopt);
}

// -- UTILS --
GLenum GLGraphicsDevice::gl_internal_format(TextureFormat format) {
    switch(format) {
        case TextureFormat::RGBA32F:
            return GL_RGBA32F;
        case TextureFormat::Depth32F:
            return GL_DEPTH_COMPONENT32F;
        default:
            assert(false);
            return GL_NONE;
    }
}

GLenum GLGraphicsDevice::gl_format(TextureFormat format) {
    switch(format) {
        case TextureFormat::RGBA32F:
            return GL_RGBA;
        case TextureFormat::Depth32F:
            return GL_DEPTH_COMPONENT;
        default:
            assert(false);
            return GL_NONE;
    }
}

GLenum GLGraphicsDevice::gl_type(TextureFormat format) {
    switch(format) {
        case TextureFormat::RGBA32F:
            return GL_FLOAT;
        case TextureFormat::Depth32F:
            return GL_FLOAT;
        default:
            assert(false);
            return GL_NONE;
    }
}

GLenum GLGraphicsDevice::gl_shader_type(ShaderStage stage) {
    switch(stage) {
        case ShaderStage::Vertex:
            return GL_VERTEX_SHADER;
        case ShaderStage::Fragment:
            return GL_FRAGMENT_SHADER;
        case ShaderStage::Compute:
            return GL_COMPUTE_SHADER;
        default:
            assert(false);
            return GL_NONE;
    }
}

GLenum GLGraphicsDevice::gl_draw_mode(PrimitiveTopology primitive_topology) {
    switch(primitive_topology) {
        case PrimitiveTopology::PointList:
            return GL_POINTS;
        case PrimitiveTopology::LineList:
            return GL_LINES;
        case PrimitiveTopology::TriangleList:
            return GL_TRIANGLES;
        default:
            assert(false);
            return GL_NONE;
    }
}

GLenum GLGraphicsDevice::gl_index_type(IndexFormat format) {
    switch(format) {
        case IndexFormat::UInt16:
            return GL_UNSIGNED_SHORT;
        case IndexFormat::UInt32:
            return GL_UNSIGNED_INT;
        default:
            assert(false);
            return GL_NONE;
    }
}

uint64_t GLGraphicsDevice::gl_index_type_size(IndexFormat format) {
    switch(format) {
        case IndexFormat::UInt16:
            return 2;
        case IndexFormat::UInt32:
            return 3;
        default:
            assert(false);
            return 0;
    }
}

uint64_t GLGraphicsDevice::gl_vertices_per_primitive(PrimitiveTopology primitive_topology) {
    switch(primitive_topology) {
        case PrimitiveTopology::PointList:
            return 1;
        case PrimitiveTopology::LineList:
            return 2;
        case PrimitiveTopology::TriangleList:
            return 3;
        default:
            assert(false);
            return 0;
    }
}

void GLGraphicsDevice::gl_get_error() {
    GLenum error;
    while((error = glGetError()) != GL_NO_ERROR) {
        const char* message;
        switch (error) {
            case GL_INVALID_ENUM:
                message = "GL_INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                message = "GL_INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                message = "GL_INVALID_OPERATION";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                message = "GL_INVALID_FRAMEBUFFER_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                message = "GL_OUT_OF_MEMORY";
                break;
            case GL_STACK_UNDERFLOW:
                message = "GL_STACK_UNDERFLOW";
                break;
            case GL_STACK_OVERFLOW:
                message = "GL_STACK_OVERFLOW";
                break;
            default:
                message = "UNKNOWN_OPENGL_ERROR";
                break;
        }
        engine::utils::throw_error(
            std::string("OpenGL error : ") +
            message + " (0x" +
            [&]() {
                std::stringstream ss;
                ss << std::hex << error;
                return ss.str();
            }() +
            ")"
        );
    }
}

// -- CONTEXT MANAGEMENT --
void GLGraphicsDevice::gl_reset_context() {
    assert(headless_context.has_value());
    gl_set_context(headless_context.value());
}

void GLGraphicsDevice::gl_set_context(GLContext& context) {
    assert(context.get_glfw_window() != nullptr);
    if(bound_context == &context) return;
    glfwMakeContextCurrent(context.get_glfw_window());
    bound_context = &context;
}

GLContext& GLGraphicsDevice::gl_get_context() {
    assert(bound_context != nullptr);
    return *bound_context;
}

// -- RESOURCES --
GLBuffer GLGraphicsDevice::gl_create_buffer(        
    uint64_t size, 
    BufferUsage usage,
    BufferFrequency frequency,
    const void* data
) {
    if(size == 0) {
        engine::utils::throw_error("Buffer size must be non-zero");
    }

    // create buffer
    GLuint handle;
    glCreateBuffers(1, &handle);
    
    // allocate storage
    GLbitfield flags = 0;
    switch(frequency) {
        case BufferFrequency::Static:
            break;
        case BufferFrequency::Dynamic:
            flags |= GL_DYNAMIC_STORAGE_BIT;
            break;
        default:
            assert(false);
    }
    assert(size > 0);
    glNamedBufferStorage(handle, size, data, flags);

    GLBuffer buf(size, usage, frequency, *this, handle);
    return buf;
}

GLTexture GLGraphicsDevice::gl_create_texture(
    TextureDimension type, 
    TextureFormat format, 
    uint32_t width, 
    uint32_t height, 
    uint32_t depth, 
    uint32_t mip_levels
) {
    if(width == 0 || height == 0 || depth == 0) {
        engine::utils::throw_error("Texture dimensions must be non-zero");
    }
    if(mip_levels == 0) {
        engine::utils::throw_error("Texture mip_levels must be non-zero");
    }

    switch(type) {
        case TextureDimension::D1:
            if(height != 1 || depth != 1) {
                engine::utils::throw_error("1D Texture needs to have unit height and depth");
            }
            break;
        case TextureDimension::D2:
            if(depth != 1) {
                engine::utils::throw_error("2D Texture needs to have unit depth");
            }
            break;
        case TextureDimension::D3:
            break;
        case TextureDimension::Cube:
            if(depth != 1) {
                engine::utils::throw_error("Cubemap Texture needs to have unit depth");
            }
            break;
        default:
            assert(false);
    }

    // create texture + allocate storage
    GLuint handle;
    GLint gl_internal_format = GLGraphicsDevice::gl_internal_format(format);
    switch(type) {
        case TextureDimension::D1:
            glCreateTextures(GL_TEXTURE_1D, 1, &handle);
            glTextureStorage1D(handle, mip_levels, gl_internal_format, width);
            break;
        case TextureDimension::D2:
            glCreateTextures(GL_TEXTURE_2D, 1, &handle);
            glTextureStorage2D(handle, mip_levels, gl_internal_format, width, height);
            break;
        case TextureDimension::D3:
            glCreateTextures(GL_TEXTURE_3D, 1, &handle);
            glTextureStorage3D(handle, mip_levels, gl_internal_format, width, height, depth);
            break;
        case TextureDimension::Cube:
            glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &handle);
            glTextureStorage2D(handle, mip_levels, gl_internal_format, width, height);
            std::cout << "Create cubemap : " << handle << std::endl;
            break;
        default:
            assert(false);
    }

    GLTexture tex(
        type, format, 
        width, height, depth,
        mip_levels,
        *this, handle
    );
    return tex;
}

GLSampler GLGraphicsDevice::gl_create_sampler(
    SamplerFilter min_filter,
    SamplerFilter mag_filter,
    SamplerMipmapFilter mipmap_filter,
    SamplerAddressMode address_u,
    SamplerAddressMode address_v,
    SamplerAddressMode address_w
) {
    // create sampler
    GLuint handle;
    glCreateSamplers(1, &handle);

    GLenum gl_min_filter;
    switch(mipmap_filter) {
        case SamplerMipmapFilter::None:
            switch(min_filter) {
                case SamplerFilter::Nearest:
                    gl_min_filter = GL_NEAREST;
                    break;
                case SamplerFilter::Linear:
                    gl_min_filter = GL_LINEAR;
                    break;
                default:
                    assert(false);
            }
            break;
        case SamplerMipmapFilter::Nearest:
            switch(min_filter) {
                case SamplerFilter::Nearest:
                    gl_min_filter = GL_NEAREST_MIPMAP_NEAREST;
                    break;
                case SamplerFilter::Linear:
                    gl_min_filter = GL_LINEAR_MIPMAP_NEAREST;
                    break;
                default:
                    assert(false);
            }
            break;
        case SamplerMipmapFilter::Linear:
            switch(min_filter) {
                case SamplerFilter::Nearest:
                    gl_min_filter = GL_NEAREST_MIPMAP_LINEAR;
                    break;
                case SamplerFilter::Linear:
                    gl_min_filter = GL_LINEAR_MIPMAP_LINEAR;
                    break;
                default:
                    assert(false);
            }
            break;
        default:
            assert(false);
    }

    GLenum gl_mag_filter;
    switch(mag_filter) {
        case SamplerFilter::Nearest:
            gl_mag_filter = GL_NEAREST;
            break;
        case SamplerFilter::Linear:
            gl_mag_filter = GL_LINEAR;
            break;
        default:
            assert(false);
    }

    auto gl_address_mode = [](SamplerAddressMode mode) -> GLenum {
        switch (mode) {
            case SamplerAddressMode::Repeat:
                return GL_REPEAT;
            case SamplerAddressMode::MirroredRepeat:
                return GL_MIRRORED_REPEAT;
            case SamplerAddressMode::ClampToEdge:
                return GL_CLAMP_TO_EDGE;
            case SamplerAddressMode::ClampToBorder:
                return GL_CLAMP_TO_BORDER;
            default:
                assert(false);
                return GL_REPEAT;
        }
    };

    glSamplerParameteri(handle, GL_TEXTURE_MIN_FILTER, gl_min_filter);
    glSamplerParameteri(handle, GL_TEXTURE_MAG_FILTER, gl_mag_filter);
    glSamplerParameteri(handle, GL_TEXTURE_WRAP_S, gl_address_mode(address_u));
    glSamplerParameteri(handle, GL_TEXTURE_WRAP_T, gl_address_mode(address_v));
    glSamplerParameteri(handle, GL_TEXTURE_WRAP_R, gl_address_mode(address_w));

    GLSampler sampler(
        min_filter,
        mag_filter,
        mipmap_filter,
        address_u,
        address_v,
        address_w,
        *this, handle
    );
    return sampler;
}

GLVertexArray GLGraphicsDevice::gl_create_vertex_array() {
    // create vertex array 
    GLuint handle;
    glCreateVertexArrays(1, &handle);

    GLVertexArray va(*this, *bound_context, handle);
    return va;
}

GLRenderbuffer GLGraphicsDevice::gl_create_renderbuffer(
    TextureFormat format,
    uint32_t width,
    uint32_t height
) {
    // create renderbuffer
    GLuint handle;
    glCreateRenderbuffers(1, &handle);

    // allocate storage
    GLint gl_internal_format = GLGraphicsDevice::gl_internal_format(format);
    glNamedRenderbufferStorage(
        handle,
        gl_internal_format,
        width,
        height
    );

    GLRenderbuffer rb(
        format,
        width, height,
        *this, handle
    );
    return rb;
}

GLFramebuffer GLGraphicsDevice::gl_create_framebuffer() {
    // create framebuffer
    GLuint handle;
    glCreateFramebuffers(1, &handle);

    GLFramebuffer fb(*this, *bound_context, handle);
    return fb;
}

GLFramebuffer GLGraphicsDevice::gl_create_window_framebuffer(
    GLWindow& window
) {
    // rendering to framebuffer 0 renders to the window in the bound context
    return GLFramebuffer(*this, window.get_context(), 0);
}

GLShader GLGraphicsDevice::gl_create_shader(
    ShaderStage stage, 
    std::string source_code,
    std::vector<uint32_t> spirv_code
) {
    // create shader
    GLenum gl_shader_type = GLGraphicsDevice::gl_shader_type(stage);
    GLuint handle = glCreateShader(gl_shader_type);

    // upload SPIR-V binary
    glShaderBinary(
        1,
        &handle,
        GL_SHADER_BINARY_FORMAT_SPIR_V,
        spirv_code.data(),
        (GLsizei) (spirv_code.size() * sizeof(uint32_t))
    );

    // compile shader 
    const char* entry_point = "main";
    glSpecializeShader(
        handle,
        entry_point,
        0,
        nullptr,
        nullptr
    );

    // check if compilation failed
    GLint status;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    if (!status) {
        GLint logLength;
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLength);
        std::string log(logLength, '\0');
        glGetShaderInfoLog(
            handle,
            logLength,
            nullptr,
            log.data()
        );
        glDeleteShader(handle);
        engine::utils::throw_error("SPIR-V shader compilation failed :\n" + log);
    }

    GLShader shader(
        stage,
        source_code,
        spirv_code,
        *this, handle
    );
    return shader;
}

GLRenderingProgram GLGraphicsDevice::gl_create_rendering_program(
    GLShader& vertex_shader, 
    GLShader& fragment_shader, 
    PrimitiveTopology primitive_topology
) {
    if(vertex_shader.get_stage() != ShaderStage::Vertex) {
        engine::utils::throw_error("Expect vertex_shader to be of type Vertex");
    }
    if(fragment_shader.get_stage() != ShaderStage::Fragment) {
        engine::utils::throw_error("Expect fragment_shader to be of type Fragment");
    }

    // create program
    GLuint handle = glCreateProgram();

    // attach shaders and link
    glAttachShader(handle, vertex_shader.handle);
    glAttachShader(handle, fragment_shader.handle);
    glLinkProgram(handle);

    GLint success;
    glGetProgramiv(handle, GL_LINK_STATUS, &success);
    if (!success) {
        GLint log_size;
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_size);
        std::string log(log_size, '\0');
        glGetProgramInfoLog(handle, log_size, nullptr, log.data());
        glDeleteProgram(handle);
        engine::utils::throw_error("Graphics program linking failed :\n" + log);
    }

    GLRenderingProgram program(
        vertex_shader,
        fragment_shader,
        primitive_topology,
        *this,
        handle
    );
    return program;
}

GLComputeProgram GLGraphicsDevice::gl_create_compute_program(
    GLShader& compute_shader
) {
    if(compute_shader.get_stage() != ShaderStage::Compute) {
        engine::utils::throw_error("Expect compute_shader to be of type Compute");
    }

    // create program
    GLuint handle = glCreateProgram();

    // attach shaders and link
    glAttachShader(handle, compute_shader.handle);
    glLinkProgram(handle);

    GLint success;
    glGetProgramiv(handle, GL_LINK_STATUS, &success);
    if (!success) {
        GLint log_size;
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_size);
        std::string log(log_size, '\0');
        glGetProgramInfoLog(handle, log_size, nullptr, log.data());
        glDeleteProgram(handle);
        engine::utils::throw_error("Compute program linking failed :\n" + log);
    }

    GLComputeProgram program(
        compute_shader,
        *this, handle
    );
    return program;
}


// -- RESOURCE DELETION --
void GLGraphicsDevice::gl_delete_buffer(GLuint gl_handle) {
    glDeleteBuffers(1, &gl_handle);
}

void GLGraphicsDevice::gl_delete_texture(GLuint gl_handle) {
    glDeleteTextures(1, &gl_handle);
}

void GLGraphicsDevice::gl_delete_sampler(GLuint gl_handle) {
    glDeleteSamplers(1, &gl_handle);
}

void GLGraphicsDevice::gl_delete_vertex_array(GLuint gl_handle) {
    glDeleteVertexArrays(1, &gl_handle);
}

void GLGraphicsDevice::gl_delete_renderbuffer(GLuint gl_handle) {
    glDeleteRenderbuffers(1, &gl_handle);
}

void GLGraphicsDevice::gl_delete_framebuffer(GLuint gl_handle) {
    glDeleteFramebuffers(1, &gl_handle);
}

void GLGraphicsDevice::gl_delete_shader(GLuint gl_handle) {
    glDeleteShader(gl_handle);
}

void GLGraphicsDevice::gl_delete_rendering_program(GLuint gl_handle) {
    glDeleteProgram(gl_handle);
}

void GLGraphicsDevice::gl_delete_compute_program(GLuint gl_handle) {
    glDeleteProgram(gl_handle);
}


// -- EXECUTION --
void GLGraphicsDevice::gl_begin() {
    assert(!executing);
    executing = true;

    // make sure nothing is bound
    // TODO
}

void GLGraphicsDevice::gl_end() {
    assert(executing);
    executing = false;

    // for now, unbind anything that is still bound
    // TODO switch to a lazy unbinding scheme where we only unbind when necessary

    // unbind uniform buffers
    for(int i = 0; i < bound_uniform_buffers.size(); i++) {
        if(!bound_uniform_buffers[i].has_value()) continue;
        glBindBufferBase(GL_UNIFORM_BUFFER, i, 0);
        bound_uniform_buffers[i] = std::nullopt;
    }

    // unbind storage buffers
    for(int i = 0; i < bound_storage_buffers.size(); i++) {
        if(!bound_storage_buffers[i].has_value()) continue;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, 0);
        bound_storage_buffers[i] = std::nullopt;
    }

    // unbind textures
    for(int i = 0; i < bound_textures.size(); i++) {
        if(!bound_textures[i].has_value()) continue;
        glBindTextureUnit(i, 0);
        bound_textures[i] = std::nullopt;
    }
    
    // unbind samplers
    for(int i = 0; i < bound_samplers.size(); i++) {
        if(!bound_samplers[i].has_value()) continue;
        glBindSampler(i, 0);
        bound_samplers[i] = std::nullopt;
    }

    // unbind vertex array
    if(bound_vertex_array.has_value()) {
        glBindVertexArray(0);
        bound_vertex_array = std::nullopt;
    }

    // unbind framebuffer
    if(bound_framebuffer.has_value()) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        bound_framebuffer = std::nullopt;
    }

    // unbind program
    if(program_bound) {
        glUseProgram(0);
        program_bound = false;
        bound_rendering_program = std::nullopt;
        bound_compute_program = std::nullopt;
    }
}

void GLGraphicsDevice::gl_bind_buffer(uint32_t slot, GLBuffer& buf) {
    assert(executing);
    assert(&buf.device == this);

    // make sure slot is in range and is unbound
    // should only be able to bind uniform buffers and storage buffers
    if(buf.get_usage() == BufferUsage::Uniform) {
        if(slot >= bound_uniform_buffers.size()) {
            engine::utils::throw_error("bind_buffer slot out of range, should be less than " + std::to_string(bound_uniform_buffers.size()));
        }
        if(bound_uniform_buffers[slot] != std::nullopt) {
            engine::utils::throw_error("buffer already bound to slot : " + std::to_string(slot));
        }
    }
    else if(buf.get_usage() == BufferUsage::Storage) {
        if(slot >= bound_storage_buffers.size()) {
            engine::utils::throw_error("bind_buffer slot out of range, should be less than " + std::to_string(bound_storage_buffers.size()));
        }
        if(bound_storage_buffers[slot] != std::nullopt) {
            engine::utils::throw_error("buffer already bound to slot : " + std::to_string(slot));
        }
    }
    else {
        engine::utils::throw_error("bind_buffer should only be able to bind uniform and storage buffers");
    }
    bound_uniform_buffers[slot] = &buf;

    // bind buffer to slot
    GLenum gl_target;
    switch(buf.get_usage()) {
        case BufferUsage::Uniform:
            gl_target = GL_UNIFORM_BUFFER;
            break;
        case BufferUsage::Storage:
            gl_target = GL_SHADER_STORAGE_BUFFER;
            break;
        default:
            assert(false);
    }
    glBindBufferBase(gl_target, slot, buf.get_handle());
}

void GLGraphicsDevice::gl_bind_texture(uint32_t slot, GLTexture& tex) {
    assert(executing);
    assert(&tex.device == this);

    // make sure slot is in range and is unbound
    if(slot >= bound_textures.size()) {
        engine::utils::throw_error("bind_texture slot out of range, should be less than " + std::to_string(bound_textures.size()));
    }
    if(bound_textures[slot] != std::nullopt) {
        engine::utils::throw_error("texture already bound to slot : " + std::to_string(slot));
    }
    bound_textures[slot] = &tex;
    
    // bind texture
    glBindTextureUnit(slot, tex.get_handle());
}

void GLGraphicsDevice::gl_bind_sampler(uint32_t slot, GLSampler& sampler) {
    assert(executing);
    assert(&sampler.device == this);

    // make sure slot is in range and unbound
    if(slot >= bound_samplers.size()) {
        // TODO
    }
    if(bound_samplers[slot] != std::nullopt) {
        // TODO
    }
    bound_samplers[slot] = &sampler;

    // bind sampler
    glBindSampler(slot, sampler.get_handle());
}

void GLGraphicsDevice::gl_bind_vertex_array(GLVertexArray& va) {
    assert(executing);
    assert(&va.device == this);
    assert(!bound_vertex_array.has_value());

    // make sure context of this vertex array matches currently bound context
    assert(bound_context != nullptr);
    if(!(va.get_context() == *bound_context)) {
        engine::utils::throw_error("vertex array context does not match currently bound context");
    }

    glBindVertexArray(va.get_handle());
    bound_vertex_array = &va;
}

void GLGraphicsDevice::gl_bind_framebuffer(GLFramebuffer& fb) {
    assert(executing);
    assert(&fb.device == this);
    assert(!bound_framebuffer.has_value());

    // make sure context of this framebuffer matches currently bound context
    assert(bound_context != nullptr);
    if(!(fb.get_context() == *bound_context)) {
        engine::utils::throw_error("framebuffer context does not match currently bound context");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fb.get_handle());
    bound_framebuffer = &fb;
}

void GLGraphicsDevice::gl_bind_rendering_program(GLRenderingProgram& program) {
    assert(executing);
    assert(&program.device == this);
    assert(!program_bound);
    assert(!bound_rendering_program.has_value());

    glUseProgram(program.get_handle());
    program_bound = true;
    bound_rendering_program = &program;
}

void GLGraphicsDevice::gl_bind_compute_program(GLComputeProgram& program) {
    assert(executing);
    assert(&program.device == this);
    assert(!program_bound);
    assert(!bound_compute_program.has_value());

    glUseProgram(program.get_handle());
    program_bound = true;
    bound_compute_program = &program;
}

void GLGraphicsDevice::gl_set_viewport(
    float min_x,
    float min_y,
    float width,
    float height,
    float min_z,
    float max_z
) {
    assert(executing);
    if(width <= 0.0f || height <= 0.0f) {
        engine::utils::throw_error("Viewport width and height must be positive");
    }
    if(min_z > max_z) {
        engine::utils::throw_error("Viewport min_z must not be greater than max_z");
    }

    glViewport(
        min_x, min_y,
        width, height
    );
    glDepthRange(min_z, max_z);
}

void GLGraphicsDevice::gl_set_scissor(
    int32_t min_x,
    int32_t min_y,
    uint32_t width,
    uint32_t height
) {
    assert(executing);
    if(width == 0 || height == 0) {
        engine::utils::throw_error("Scissor width and height must be non-zero");
    }

    // TODO
    assert(false);
}

void GLGraphicsDevice::gl_draw_arrays(
    uint32_t primitive_count,
    uint32_t first_primitive
) {
    assert(executing);
    assert(bound_vertex_array.has_value());
    assert(bound_framebuffer.has_value());
    assert(bound_rendering_program.has_value());
    assert(bound_vertex_array.value() != nullptr);
    assert(bound_framebuffer.value() != nullptr);
    assert(bound_rendering_program.value() != nullptr);

    PrimitiveTopology topology = bound_rendering_program.value()->get_primitive_topology();
    GLenum gl_draw_mode = GLGraphicsDevice::gl_draw_mode(topology);
    uint64_t vertices_per_primitive = GLGraphicsDevice::gl_vertices_per_primitive(topology);

    glDrawArrays(
        gl_draw_mode,
        first_primitive * vertices_per_primitive,
        primitive_count * vertices_per_primitive
    );
}

void GLGraphicsDevice::gl_draw_elements(
    uint32_t primitive_count,
    uint32_t first_primitive
) {
    assert(executing);
    assert(bound_vertex_array.has_value());
    assert(bound_framebuffer.has_value());
    assert(bound_rendering_program.has_value());
    assert(bound_vertex_array.value() != nullptr);
    assert(bound_framebuffer.value() != nullptr);
    assert(bound_rendering_program.value() != nullptr);
    if(!bound_vertex_array.value()->has_index_buffer()) {
        engine::utils::throw_error("gl_draw_elements expects the bound vertex array to have a bound index buffer");
    }

    PrimitiveTopology topology = bound_rendering_program.value()->get_primitive_topology();
    GLenum gl_draw_mode = GLGraphicsDevice::gl_draw_mode(topology);
    uint64_t vertices_per_primitive = gl_vertices_per_primitive(topology);
    
    IndexFormat index_format = bound_vertex_array.value()->get_index_format();
    GLenum gl_index_type = GLGraphicsDevice::gl_index_type(index_format);
    uint64_t index_type_size = GLGraphicsDevice::gl_index_type_size(index_format);

    glDrawElements(
        gl_draw_mode,
        primitive_count * vertices_per_primitive,
        gl_index_type,
        reinterpret_cast<const void*>(
            first_primitive * vertices_per_primitive * index_type_size
        )
    );
}

void GLGraphicsDevice::gl_draw_arrays_instanced(
    uint32_t primitive_count,
    uint32_t instance_count,
    uint32_t first_primitive,
    uint32_t first_instance
) {
    assert(executing);
    assert(bound_vertex_array.has_value());
    assert(bound_framebuffer.has_value());
    assert(bound_rendering_program.has_value());
    assert(bound_vertex_array.value() != nullptr);
    assert(bound_framebuffer.value() != nullptr);
    assert(bound_rendering_program.value() != nullptr);

    PrimitiveTopology topology = bound_rendering_program.value()->get_primitive_topology();
    GLenum gl_draw_mode = GLGraphicsDevice::gl_draw_mode(topology);
    uint64_t vertices_per_primitive = GLGraphicsDevice::gl_vertices_per_primitive(topology);

    glDrawArraysInstancedBaseInstance(
        gl_draw_mode,
        first_primitive * vertices_per_primitive,
        primitive_count * vertices_per_primitive,
        instance_count,
        first_instance
    );
}

void GLGraphicsDevice::gl_draw_elements_instanced(
    uint32_t primitive_count,
    uint32_t instance_count,
    uint32_t first_primitive,
    uint32_t first_instance
) {
    assert(executing);
    assert(bound_vertex_array.has_value());
    assert(bound_framebuffer.has_value());
    assert(bound_rendering_program.has_value());
    assert(bound_vertex_array.value() != nullptr);
    assert(bound_framebuffer.value() != nullptr);
    assert(bound_rendering_program.value() != nullptr);
    if(!bound_vertex_array.value()->has_index_buffer()) {
        engine::utils::throw_error("gl_draw_elements_instanced expects the bound vertex array to have a bound index buffer");
    }
    
    PrimitiveTopology topology = bound_rendering_program.value()->get_primitive_topology();
    GLenum gl_draw_mode = GLGraphicsDevice::gl_draw_mode(topology);
    uint64_t vertices_per_primitive = gl_vertices_per_primitive(topology);
    
    IndexFormat index_format = bound_vertex_array.value()->get_index_format();
    GLenum gl_index_type = GLGraphicsDevice::gl_index_type(index_format);
    uint64_t index_type_size = GLGraphicsDevice::gl_index_type_size(index_format);

    glDrawElementsInstancedBaseInstance(
        gl_draw_mode,
        primitive_count * vertices_per_primitive,
        gl_index_type,
        reinterpret_cast<const void*>(
            first_primitive * vertices_per_primitive * index_type_size
        ),
        instance_count,
        first_instance
    );
}

void GLGraphicsDevice::gl_dispatch(uint32_t x, uint32_t y, uint32_t z) {
    assert(executing);
    
    // TODO
    assert(false);
}


// -- EXTERNAL API --
std::unique_ptr<engine::graphics::Buffer> GLGraphicsDevice::create_buffer(
    uint64_t size, 
    BufferUsage usage,
    BufferFrequency frequency,
    const void* data
) {
    return std::make_unique<GLBuffer>(std::move(gl_create_buffer(
        size, 
        usage,
        frequency,
        data
    )));
}

std::unique_ptr<engine::graphics::Texture> GLGraphicsDevice::create_texture(
    TextureDimension type, 
    TextureFormat format, 
    uint32_t width, 
    uint32_t height, 
    uint32_t depth, 
    uint32_t mip_levels
) {
    return std::make_unique<GLTexture>(std::move(gl_create_texture(
        type,
        format,
        width,
        height,
        depth,
        mip_levels
    )));
}

std::unique_ptr<engine::graphics::Sampler> GLGraphicsDevice::create_sampler(
    SamplerFilter min_filter,
    SamplerFilter mag_filter,
    SamplerMipmapFilter mipmap_filter,
    SamplerAddressMode address_u,
    SamplerAddressMode address_v,
    SamplerAddressMode address_w
) {
    return std::make_unique<GLSampler>(std::move(gl_create_sampler(
        min_filter,
        mag_filter,
        mipmap_filter,
        address_u,
        address_v,
        address_w
    )));
}

std::unique_ptr<engine::graphics::TextureRenderTarget> GLGraphicsDevice::create_render_target(
    uint32_t width,
    uint32_t height
) {
    return std::make_unique<GLTextureRenderTarget>(std::move(GLTextureRenderTarget(
        width,
        height,
        *this,
        gl_create_framebuffer()
    )));
}

std::unique_ptr<engine::graphics::WindowRenderTarget> GLGraphicsDevice::create_window_render_target(
    Window& window
) {
    assert(dynamic_cast<GLWindow*>(&window) != nullptr);
    GLWindow& gl_window = dynamic_cast<GLWindow&>(window);
    auto retval = std::make_unique<GLWindowRenderTarget>(std::move(GLWindowRenderTarget(
        *this,
        gl_window
    )));
    retval->validate();
    return retval;
}

std::unique_ptr<engine::graphics::RenderingProgram> GLGraphicsDevice::create_rendering_program(
    engine::graphics::Shader& _vertex, 
    engine::graphics::Shader& _fragment,
    PrimitiveTopology primitive_topology
) {
    if(!dynamic_cast<engine::graphics::opengl::GLShader*>(&_vertex)) {
        engine::utils::throw_error("create_rendering_program vertex shader needs to be a GLShader");
    }
    if(!dynamic_cast<engine::graphics::opengl::GLShader*>(&_fragment)) {
        engine::utils::throw_error("create_rendering_program fragment shader needs to be a GLShader");
    }
    engine::graphics::opengl::GLShader& vertex = dynamic_cast<engine::graphics::opengl::GLShader&>(_vertex);
    engine::graphics::opengl::GLShader& fragment = dynamic_cast<engine::graphics::opengl::GLShader&>(_fragment);
    assert(&vertex.device == this);
    assert(&fragment.device == this);

    using ShaderStage = engine::graphics::ShaderStage;
    if(vertex.get_stage() != ShaderStage::Vertex) {
        engine::utils::throw_error("create_rendering_program vertex shader should be stage Vertex");
    }
    if(fragment.get_stage() != ShaderStage::Fragment) {
        engine::utils::throw_error("create_rendering_program fragment shader should be stage Fragment");
    }

    // create a GPU program
    return std::make_unique<GLRenderingProgram>(std::move(gl_create_rendering_program(
        vertex,
        fragment,
        primitive_topology
    )));
}

std::unique_ptr<engine::graphics::ComputeProgram> GLGraphicsDevice::create_compute_program(
    engine::graphics::Shader& _compute
) {
    // TODO
}

std::unique_ptr<engine::graphics::Shader> GLGraphicsDevice::create_shader(
    const std::string_view& source_code,
    ShaderStage stage,
    const fs::path& source_path
) {
    // determine shaderc kind
    shaderc_shader_kind sc_kind;
    switch(stage) {
        case ShaderStage::Vertex:
            sc_kind = shaderc_glsl_vertex_shader;
            break;
        case ShaderStage::Fragment:
            sc_kind = shaderc_glsl_fragment_shader;
            break;
        case ShaderStage::Compute:
            sc_kind = shaderc_glsl_compute_shader;
            break;
        default:
            assert(false);
    }

    // create a compiler
    shaderc_compiler_t sc_compiler = shaderc_compiler_initialize();
    if(!sc_compiler) {
        engine::utils::throw_error("Failed to initialize shaderc");
    }

    // compile into SPIR-V
    shaderc_compile_options_t sc_options = shaderc_compile_options_initialize();
    shaderc_compile_options_set_target_env(
        sc_options,
        shaderc_target_env_opengl,
        shaderc_env_version_opengl_4_5
    );
    shaderc_compile_options_set_target_spirv(
        sc_options,
        shaderc_spirv_version_1_0
    );

    shaderc_compilation_result_t sc_result = shaderc_compile_into_spv(
        sc_compiler,
        source_code.data(),
        source_code.size(),
        sc_kind,
        source_path.string().c_str(),
        "main",
        sc_options
    );
    shaderc_compile_options_release(sc_options);

    if(shaderc_result_get_compilation_status(sc_result) != shaderc_compilation_status_success) {
        std::string error = shaderc_result_get_error_message(sc_result);
        shaderc_result_release(sc_result);
        shaderc_compiler_release(sc_compiler);
        engine::utils::throw_error("Shader compilation failed : \n" + error);
    }

    // copy SPIR-V 
    const char* spirv = shaderc_result_get_bytes(sc_result);
    size_t spirv_size = shaderc_result_get_length(sc_result);
    assert(spirv_size % 4 == 0);    // spirv should always be castable to a uint32_t*
    std::vector<uint32_t> spirv_code;
    spirv_code.assign((uint32_t*) spirv, (uint32_t*) spirv + spirv_size / sizeof(uint32_t));

    shaderc_result_release(sc_result);
    shaderc_compiler_release(sc_compiler);

    // create shader
    return std::make_unique<GLShader>(std::move(gl_create_shader(
        stage,
        std::string(source_code),
        spirv_code
    )));
}

std::unique_ptr<engine::graphics::Window> GLGraphicsDevice::create_window(
    uint32_t width,
    uint32_t height,
    const char* title
) {
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* glfw_window = glfwCreateWindow(
        width,
        height,
        title,
        nullptr,
        headless_context->get_glfw_window()
    );
    if(glfw_window == nullptr) {
        glfwTerminate();
        engine::utils::throw_error("Failed to create GLFW window");
    }
    glfwShowWindow(glfw_window);

    GLContext gl_context(*this, glfw_window);
    return std::make_unique<GLWindow>(std::move(GLWindow(
        *this,
        std::move(gl_context)
    )));
}

void GLGraphicsDevice::present(Window& window) {
    assert(dynamic_cast<GLWindow*>(&window) != nullptr);
    GLWindow& gl_window = dynamic_cast<GLWindow&>(window);
    gl_window.swap_buffers();
}

uint32_t GLGraphicsDevice::get_vertex_format_location_count(VertexFormat format) {
    switch(format) {
        case VertexFormat::Float32x1:
            return 1;
        case VertexFormat::Float32x2:
            return 1;
        case VertexFormat::Float32x3:
            return 1;
        case VertexFormat::Float32x4:
            return 1;
        case VertexFormat::Float32x4x4:
            return 4;
        default:
            assert(false);
    }
}

}
