#include <engine/graphics/opengl/GraphicsDevice.h>
#include <stdexcept>
#include <cassert>
#include <utility>
#include <iostream>
#include <cstring>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace engine::graphics::opengl {

GraphicsDevice* GraphicsDevice::create(uint32_t width, uint32_t height, const char* title) {
    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "GLFW error " << error << ": " << description << std::endl;
    });

    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    if(!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    // std::cout << "GLFW platform : " << glfwGetPlatform() << std::endl;
    
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
        nullptr
    );
    if(glfw_window == nullptr) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwShowWindow(glfw_window);

    glfwMakeContextCurrent(glfw_window);
    if(gladLoadGL() == 0) {
        glfwDestroyWindow(glfw_window);
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLAD");
    }

    Window window(glfw_window);
    GraphicsDevice* device = new GraphicsDevice(std::move(window));

    // rendering to framebuffer with handle 0 renders to the window
    RenderTarget window_render_target;
    window_render_target.handle = 0;
    device->window_render_target = window_render_target;

    return device;
}

GraphicsDevice::GraphicsDevice(Window&& _window) : window(std::move(_window)) {
    // grab capabilities
    GLint gl_max_texture_slots;
    GLint gl_max_uniform_buffer_slots;
    GLint gl_max_storage_buffer_slots;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &gl_max_texture_slots);
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &gl_max_uniform_buffer_slots);
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &gl_max_storage_buffer_slots);

    assert(gl_max_texture_slots > 0);
    assert(gl_max_uniform_buffer_slots > 0);
    assert(gl_max_storage_buffer_slots > 0);

    // alloc stuff for execution state
    bound_textures = std::vector<GLint>((size_t) gl_max_texture_slots, 0);
    bound_buffers = std::vector<std::pair<GLint, BufferUsage>>((size_t) gl_max_storage_buffer_slots, {0, {}});
}

// -- RESOURCES --
Buffer GraphicsDevice::create_buffer(uint64_t size, BufferMemory memory) {
    if(size == 0) {
        throw std::runtime_error("Buffer size must be non-zero");
    }

    // create buffer
    GLuint handle;
    glGenBuffers(1, &handle);

    // allocate storage
    GLbitfield flags = 0;
    switch(memory) {
    case BufferMemory::Read:
        flags |= GL_MAP_READ_BIT;
        break;
    case BufferMemory::Write:
        flags |= GL_MAP_WRITE_BIT;
        break;
    default:
        assert(false);
    }
    assert(size > 0);
    glNamedBufferStorage(handle, size, nullptr, flags);
    
    Buffer buf;
    buf.handle = handle;
    buf.device = this;
    buf.size = size;

    return buf;
}

Texture GraphicsDevice::create_texture(
    TextureType type, 
    TextureFormat format, 
    uint32_t width, 
    uint32_t height, 
    uint32_t depth, 
    uint32_t mip_levels
) {
    if(width == 0 || height == 0 || depth == 0) {
        throw std::runtime_error("Texture dimensions must be non-zero");
    }
    if(mip_levels == 0) {
        throw std::runtime_error("Texture mip_levels must be non-zero");
    }

    switch(type) {
    case TextureType::D1:
        if(height != 1 || depth != 1) {
            throw std::runtime_error("1D Texture needs to have unit height and depth");
        }
        break;
    case TextureType::D2:
        if(depth != 1) {
            throw std::runtime_error("2D Texture needs to have unit depth");
        }
        break;
    case TextureType::D3:
        break;
    case TextureType::Cube:
        if(depth != 1) {
            throw std::runtime_error("Cubemap Texture needs to have unit depth");
        }
        break;
    default:
        assert(false);
    }

    // create texture
    GLuint handle;
    glGenTextures(1, &handle);

    // determine internal_format
    GLint gl_internal_format;
    switch(format) {
    case TextureFormat::RGBA16F:
        gl_internal_format = GL_RGBA16F;
        break;
    case TextureFormat::RGBA32F:
        gl_internal_format = GL_RGBA32F;
        break;
    default:
        assert(false);
    }

    // allocate storage
    switch(type) {
    case TextureType::D1:
        glTextureStorage1D(handle, mip_levels, gl_internal_format, width);
        break;
    case TextureType::D2:
        glTextureStorage2D(handle, mip_levels, gl_internal_format, width, height);
        break;
    case TextureType::D3:
        glTextureStorage3D(handle, mip_levels, gl_internal_format, width, height, depth);
        break;
    case TextureType::Cube:
        glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
        glTexStorage2D(GL_TEXTURE_CUBE_MAP, mip_levels, gl_internal_format, width, height);
        break;
    default:
        assert(false);
    }

    Texture tex;
    tex.handle = handle;
    tex.device = this;
    tex.type = type;
    tex.format = format;
    tex.width = width;
    tex.height = height;
    tex.depth = depth;
    tex.mip_levels = mip_levels;

    return tex;
}

RenderTarget GraphicsDevice::create_render_target() {
    // create framebuffer
    GLuint handle;
    glCreateFramebuffers(1, &handle);

    RenderTarget target;
    target.handle = handle;
    target.device = this;

    return target;
}

Shader GraphicsDevice::create_shader(ShaderType type, const std::string& source_code) {
    // determine shader type
    GLenum gl_shader_type;
    switch(type) {
    case ShaderType::Vertex:
        gl_shader_type = GL_VERTEX_SHADER;
        break;
    case ShaderType::Fragment:
        gl_shader_type = GL_FRAGMENT_SHADER;
        break;
    case ShaderType::Compute:
        gl_shader_type = GL_COMPUTE_SHADER;
        break;
    default:
        assert(false);
    }

    // create shader
    GLuint handle = glCreateShader(gl_shader_type);

    // compile shader source
    {
        const char* source = source_code.c_str();
        glShaderSource(handle, 1, &source, nullptr);
        glCompileShader(handle);

        GLint success;
        glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
        if(!success) {
            GLint log_size;
            glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_size);
            std::string log(log_size, '\0');
            glGetShaderInfoLog(handle, log_size, nullptr, log.data());
            glDeleteShader(handle);
            throw std::runtime_error("Shader compilation failed :\n" + log);
        }
    }

    Shader shader;
    shader.handle = handle;
    shader.device = this;

    return shader;
}

GraphicsProgram GraphicsDevice::create_graphics_pipeline(
    Shader& vertex_shader, 
    Shader& fragment_shader, 
    PrimitiveTopology primitive_topology
) {
    if(vertex_shader.type != ShaderType::Vertex) {
        throw std::runtime_error("Expect vertex_shader to be of type Vertex");
    }
    if(fragment_shader.type != ShaderType::Fragment) {
        throw std::runtime_error("Expect fragment_shader to be of type Fragment");
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
        throw std::runtime_error("Graphics pipeline linking failed :\n" + log);
    }

    GraphicsProgram pipeline;
    pipeline.handle = handle;
    pipeline.device = this;
    pipeline.primitive_topology = primitive_topology;

    return pipeline;
}

ComputeProgram GraphicsDevice::create_compute_pipeline(Shader& compute_shader) {
    if(compute_shader.type != ShaderType::Compute) {
        throw std::runtime_error("Expect compute_shader to be of type Compute");
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
        throw std::runtime_error("Compute pipeline linking failed :\n" + log);
    }

    ComputeProgram pipeline;
    pipeline.handle = handle;
    pipeline.device = this;

    return pipeline;
}


// -- RESOURCE MANAGEMENT --
void GraphicsDevice::upload_buffer(
    Buffer& dst,
    const void* data,
    uint64_t size,
    uint64_t offset
) {
    assert(dst.device == this);
    if(data == nullptr) {
        throw std::runtime_error("upload_buffer data cannot be nullptr");
    }
    if(size + offset > dst.get_size()) {
        throw std::runtime_error("upload_buffer going out of bounds in buffer");
    }

    GLuint gl_handle = dst.handle;
    glNamedBufferSubData(gl_handle, offset, size, data);
}

void GraphicsDevice::copy_buffer(
    Buffer& src, 
    Buffer& dst,
    uint64_t size,
    uint64_t src_offset,
    uint64_t dst_offset
) {
    assert(src.device == this);
    assert(dst.device == this);
    if(size + src_offset > src.get_size()) {
        throw std::runtime_error("copy_buffer going out of bounds in src");
    }
    if(size + dst_offset > dst.get_size()) {
        throw std::runtime_error("copy_buffer going out of bounds in dst");
    }
    
    GLuint gl_src_handle = src.handle;
    GLuint gl_dst_handle = dst.handle;
    glCopyNamedBufferSubData(
        gl_src_handle,
        gl_dst_handle,
        src_offset,
        dst_offset,
        size
    );
}

void GraphicsDevice::upload_texture(
    Texture& texture,
    const void* data,
    uint32_t mip_level,
    uint32_t cube_face
) {
    assert(texture.device == this);
    if(data == nullptr) {
        throw std::runtime_error("upload_texture data cannot be nullptr");
    }
    if(mip_level >= texture.get_mip_levels()) {
        throw std::runtime_error("upload_texture mip_level out of range");
    }
    if(texture.get_type() == TextureType::Cube) {
        if(cube_face > 6) {
            throw std::runtime_error("upload_texture cube_face should be < 6");
        }
    }
    else {
        if(cube_face > 0) {
            throw std::runtime_error("upload_texture cube_face should be 0 for non-cubemap texture");
        }
    }

    GLuint gl_handle = texture.handle;
    uint64_t data_size = texture.get_size(mip_level);
    
    GLenum gl_format;
    GLenum gl_type;
    switch(texture.get_format()) {
    case TextureFormat::RGBA16F:
        gl_format = GL_RGBA;
        gl_type = GL_FLOAT;
        break;
    case TextureFormat::RGBA32F:
        gl_format = GL_RGBA;
        gl_type = GL_FLOAT;
        break;
    }

    switch(texture.get_type()) {
    case TextureType::D1:
        glTextureSubImage1D(
            gl_handle,
            mip_level,
            0,
            texture.get_width(mip_level),
            gl_format,
            gl_type,
            data
        );
        break;
    case TextureType::D2:
        glTextureSubImage2D(
            gl_handle,
            mip_level,
            0, 0,
            texture.get_width(mip_level),
            texture.get_height(mip_level),
            gl_format,
            gl_type,
            data
        );
        break;
    case TextureType::D3:
        glTextureSubImage3D(
            gl_handle,
            mip_level,
            0, 0, 0,
            texture.get_width(mip_level),
            texture.get_height(mip_level),
            texture.get_depth(mip_level),
            gl_format,
            gl_type,
            data
        );
        break;
    case TextureType::Cube:
        assert(cube_face < 6);
        glBindTexture(GL_TEXTURE_CUBE_MAP, gl_handle);
        glTexSubImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + cube_face,
            mip_level,
            0, 0,
            texture.get_width(mip_level),
            texture.get_height(mip_level),
            gl_format,
            gl_type,
            data
        );
        break;
    }
}

void GraphicsDevice::generate_mipmaps(Texture& texture) {
    assert(texture.device == this);
    
    GLuint gl_handle = texture.handle;
    glGenerateTextureMipmap(gl_handle);
}


// -- RESOURCE DELETION --
void GraphicsDevice::delete_buffer(uint64_t handle) {
    GLuint gl_handle = handle;
    glDeleteBuffers(1, &gl_handle);
}

void GraphicsDevice::delete_texture(uint64_t handle) {
    GLuint gl_handle = handle;
    glDeleteTextures(1, &gl_handle);
}

void GraphicsDevice::delete_render_target(uint64_t handle) {
    GLuint gl_handle = handle;
    glDeleteFramebuffers(1, &gl_handle);
}

void GraphicsDevice::delete_shader(uint64_t handle) {
    GLuint gl_handle = handle;
    glDeleteShader(gl_handle);
}

void GraphicsDevice::delete_graphics_pipeline(uint64_t handle) {
    GLuint gl_handle = handle;
    glDeleteProgram(gl_handle);
}

void GraphicsDevice::delete_compute_pipeline(uint64_t handle) {
    GLuint gl_handle = handle;
    glDeleteProgram(gl_handle);
}


// -- EXECUTION --
void GraphicsDevice::begin() {
    assert(!executing);
    executing = true;

    // reset viewport and scissor
    uint32_t window_width = get_window().get_width();
    uint32_t window_height = get_window().get_height();
    glViewport(
        0, 0,
        window_width, window_height
    );
    glDepthRange(0.0, 1.0);

    // make sure nothing is bound
    // TODO
}

void GraphicsDevice::end() {
    assert(executing);
    executing = false;

    // for now, unbind anything that is still bound
    // will probably want to switch to a lazy unbinding scheme later
    for(int i = 0; i < max_bound_textures; i++) {
        // see if this slot is bound
        if(bound_textures[i] == 0) {
            continue;
        }

        // unbind this slot
        glBindTextureUnit(i, 0);
        bound_textures[i] = 0;
    }
    for(int i = 0; i < max_bound_buffers; i++) {
        // see if this slot is bound
        if(bound_buffers[i].first == 0) {
            continue;
        }
        BufferUsage usage = bound_buffers[i].second;

        // unbind this slot
        switch(usage) {
        case BufferUsage::Vertex:
            
            break;
        case BufferUsage::Index:

            break;
        case BufferUsage::Uniform:

            break;
        case BufferUsage::Storage:
        
            break;
        default:
            assert(false);
        }
    }
    if(bound_framebuffer != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        bound_framebuffer = 0;
    }
    if(bound_program != 0) {
        glUseProgram(0);
        bound_program = 0;
    }
}

void GraphicsDevice::bind_texture(uint32_t slot, Texture& tex) {
    assert(executing);
    assert(tex.device == this);
    if(slot >= max_bound_textures) {
        throw std::runtime_error("OpenGL bind_texture slot should be less than " + std::to_string(slot));
    }

    glBindTextureUnit(slot, tex.handle);
}

void GraphicsDevice::bind_buffer(uint32_t slot, BufferUsage usage, Buffer& buf) {
    assert(executing);
    assert(buf.device == this);
    if(slot >= max_bound_buffers) {
        throw std::runtime_error("OpenGL bind_buffer slot should be less than " + std::to_string(slot));
    }

    switch(usage) {
    case BufferUsage::Vertex:
        
        break;
    case BufferUsage::Index:

        break;
    case BufferUsage::Uniform:

        break;
    case BufferUsage::Storage:
    
        break;
    default:
        assert(false);
    }
}

void GraphicsDevice::bind_render_target(RenderTarget& tgt) {
    assert(executing);
    assert(tgt.device == this);

    GLuint gl_handle = tgt.handle;
    glBindFramebuffer(GL_FRAMEBUFFER, gl_handle);
}

void GraphicsDevice::bind_graphics_pipeline(GraphicsProgram& pipeline) {
    assert(executing);
    assert(pipeline.device == this);

    // TODO
}

void GraphicsDevice::bind_compute_pipeline(ComputeProgram& pipeline) {
    assert(executing);
    assert(pipeline.device == this);

    // TODO
}

void GraphicsDevice::set_viewport(
    float min_x,
    float min_y,
    float width,
    float height,
    float min_z,
    float max_z
) {
    assert(executing);
    if(width <= 0.0f || height <= 0.0f) {
        throw std::runtime_error("Viewport width and height must be positive");
    }
    if(min_z > max_z) {
        throw std::runtime_error("Viewport min_z must not be greater than max_z");
    }

    glViewport(
        min_x, min_y,
        width, height
    );
    glDepthRange(min_z, max_z);
}

void GraphicsDevice::set_scissor(
    int32_t min_x,
    int32_t min_y,
    uint32_t width,
    uint32_t height
) {
    assert(executing);
    if(width == 0 || height == 0) {
        throw std::runtime_error("Scissor width and height must be non-zero");
    }

    // TODO
}

void GraphicsDevice::draw(uint32_t count, uint32_t offset) {
    assert(executing);

    // TODO
}

void GraphicsDevice::dispatch(uint32_t x, uint32_t y, uint32_t z) {
    assert(executing);
    
    // TODO
}

}
