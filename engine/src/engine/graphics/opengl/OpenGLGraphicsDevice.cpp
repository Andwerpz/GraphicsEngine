#include <engine/graphics/opengl/OpenGLGraphicsDevice.h>
#include <engine/pipeline/opengl/Pipeline.h>
#include <engine/pipeline/opengl/Shader.h>

#include <stdexcept>
#include <cassert>
#include <utility>
#include <iostream>
#include <cstring>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shaderc/shaderc.h>

#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

namespace engine::graphics::opengl {

std::unique_ptr<engine::graphics::GraphicsDevice> OpenGLGraphicsDevice::create(uint32_t width, uint32_t height, const char* title) {
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
    std::unique_ptr<OpenGLGraphicsDevice> device = std::make_unique<OpenGLGraphicsDevice>(std::move(window));

    return device;
}

OpenGLGraphicsDevice::OpenGLGraphicsDevice(Window&& _window) : window(std::move(_window)) {
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
    bound_buffers = std::vector<std::pair<GLint, GLBufferUsage>>((size_t) gl_max_storage_buffer_slots, {0, {}});
}

// -- RESOURCES --
GLBuffer OpenGLGraphicsDevice::gl_create_buffer(uint64_t size, GLBufferMemory memory) {
    if(size == 0) {
        throw std::runtime_error("Buffer size must be non-zero");
    }

    // create buffer
    GLuint handle;
    glGenBuffers(1, &handle);

    // allocate storage
    GLbitfield flags = 0;
    switch(memory) {
    case GLBufferMemory::Read:
        flags |= GL_MAP_READ_BIT;
        break;
    case GLBufferMemory::Write:
        flags |= GL_MAP_WRITE_BIT;
        break;
    default:
        assert(false);
    }
    assert(size > 0);
    glNamedBufferStorage(handle, size, nullptr, flags);
    
    GLBuffer buf(*this, handle, size, memory);
    return buf;
}

GLTexture OpenGLGraphicsDevice::gl_create_texture(
    GLTextureType type, 
    GLTextureFormat format, 
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
    case GLTextureType::D1:
        if(height != 1 || depth != 1) {
            throw std::runtime_error("1D Texture needs to have unit height and depth");
        }
        break;
    case GLTextureType::D2:
        if(depth != 1) {
            throw std::runtime_error("2D Texture needs to have unit depth");
        }
        break;
    case GLTextureType::D3:
        break;
    case GLTextureType::Cube:
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
    case GLTextureFormat::RGBA16F:
        gl_internal_format = GL_RGBA16F;
        break;
    case GLTextureFormat::RGBA32F:
        gl_internal_format = GL_RGBA32F;
        break;
    default:
        assert(false);
    }

    // allocate storage
    switch(type) {
    case GLTextureType::D1:
        glTextureStorage1D(handle, mip_levels, gl_internal_format, width);
        break;
    case GLTextureType::D2:
        glTextureStorage2D(handle, mip_levels, gl_internal_format, width, height);
        break;
    case GLTextureType::D3:
        glTextureStorage3D(handle, mip_levels, gl_internal_format, width, height, depth);
        break;
    case GLTextureType::Cube:
        glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
        glTexStorage2D(GL_TEXTURE_CUBE_MAP, mip_levels, gl_internal_format, width, height);
        break;
    default:
        assert(false);
    }

    GLTexture tex(
        *this, handle,
        type, format, 
        width, height, depth,
        mip_levels
    );
    return tex;
}

GLFramebuffer OpenGLGraphicsDevice::gl_create_framebuffer() {
    // create framebuffer
    GLuint handle;
    glCreateFramebuffers(1, &handle);

    GLFramebuffer target(*this, handle);
    return target;
}

GLShader OpenGLGraphicsDevice::gl_create_shader(GLShaderType type, const std::string& source_code) {
    // determine shader type
    GLenum gl_shader_type;
    switch(type) {
    case GLShaderType::Vertex:
        gl_shader_type = GL_VERTEX_SHADER;
        break;
    case GLShaderType::Fragment:
        gl_shader_type = GL_FRAGMENT_SHADER;
        break;
    case GLShaderType::Compute:
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

    GLShader shader(*this, handle, type);
    return shader;
}

GLGraphicsProgram OpenGLGraphicsDevice::gl_create_graphics_pipeline(
    GLShader& vertex_shader, 
    GLShader& fragment_shader, 
    PrimitiveTopology primitive_topology
) {
    if(vertex_shader.type != GLShaderType::Vertex) {
        throw std::runtime_error("Expect vertex_shader to be of type Vertex");
    }
    if(fragment_shader.type != GLShaderType::Fragment) {
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

    GLGraphicsProgram pipeline(*this, handle, primitive_topology);
    return pipeline;
}

GLComputeProgram OpenGLGraphicsDevice::gl_create_compute_pipeline(GLShader& compute_shader) {
    if(compute_shader.type != GLShaderType::Compute) {
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

    GLComputeProgram pipeline(*this, handle);
    return pipeline;
}


// -- RESOURCE MANAGEMENT --
void OpenGLGraphicsDevice::gl_upload_buffer(
    GLBuffer& dst,
    const void* data,
    uint64_t size,
    uint64_t offset
) {
    assert(&dst.device == this);
    if(data == nullptr) {
        throw std::runtime_error("upload_buffer data cannot be nullptr");
    }
    if(size + offset > dst.get_size()) {
        throw std::runtime_error("upload_buffer going out of bounds in buffer");
    }

    GLuint gl_handle = dst.handle;
    glNamedBufferSubData(gl_handle, offset, size, data);
}

void OpenGLGraphicsDevice::gl_copy_buffer(
    GLBuffer& src, 
    GLBuffer& dst,
    uint64_t size,
    uint64_t src_offset,
    uint64_t dst_offset
) {
    assert(&src.device == this);
    assert(&dst.device == this);
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

void OpenGLGraphicsDevice::gl_upload_texture(
    GLTexture& texture,
    const void* data,
    uint32_t mip_level,
    uint32_t cube_face
) {
    assert(&texture.device == this);
    if(data == nullptr) {
        throw std::runtime_error("upload_texture data cannot be nullptr");
    }
    if(mip_level >= texture.get_mip_levels()) {
        throw std::runtime_error("upload_texture mip_level out of range");
    }
    if(texture.get_type() == GLTextureType::Cube) {
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
    case GLTextureFormat::RGBA16F:
        gl_format = GL_RGBA;
        gl_type = GL_FLOAT;
        break;
    case GLTextureFormat::RGBA32F:
        gl_format = GL_RGBA;
        gl_type = GL_FLOAT;
        break;
    }

    switch(texture.get_type()) {
    case GLTextureType::D1:
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
    case GLTextureType::D2:
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
    case GLTextureType::D3:
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
    case GLTextureType::Cube:
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

void OpenGLGraphicsDevice::gl_generate_mipmaps(GLTexture& texture) {
    assert(&texture.device == this);
    
    GLuint gl_handle = texture.handle;
    glGenerateTextureMipmap(gl_handle);
}


// -- RESOURCE DELETION --
void OpenGLGraphicsDevice::gl_delete_buffer(uint64_t handle) {
    GLuint gl_handle = handle;
    glDeleteBuffers(1, &gl_handle);
}

void OpenGLGraphicsDevice::gl_delete_texture(uint64_t handle) {
    GLuint gl_handle = handle;
    glDeleteTextures(1, &gl_handle);
}

void OpenGLGraphicsDevice::gl_delete_framebuffer(uint64_t handle) {
    GLuint gl_handle = handle;
    glDeleteFramebuffers(1, &gl_handle);
}

void OpenGLGraphicsDevice::gl_delete_shader(uint64_t handle) {
    GLuint gl_handle = handle;
    glDeleteShader(gl_handle);
}

void OpenGLGraphicsDevice::gl_delete_graphics_pipeline(uint64_t handle) {
    GLuint gl_handle = handle;
    glDeleteProgram(gl_handle);
}

void OpenGLGraphicsDevice::gl_delete_compute_pipeline(uint64_t handle) {
    GLuint gl_handle = handle;
    glDeleteProgram(gl_handle);
}


// -- EXECUTION --
void OpenGLGraphicsDevice::gl_begin() {
    assert(!executing);
    executing = true;

    // reset viewport and scissor
    uint32_t window_width = gl_get_window().get_width();
    uint32_t window_height = gl_get_window().get_height();
    glViewport(
        0, 0,
        window_width, window_height
    );
    glDepthRange(0.0, 1.0);

    // make sure nothing is bound
    // TODO
}

void OpenGLGraphicsDevice::gl_end() {
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
        GLBufferUsage usage = bound_buffers[i].second;

        // unbind this slot
        switch(usage) {
        case GLBufferUsage::Vertex:
            
            break;
        case GLBufferUsage::Index:

            break;
        case GLBufferUsage::Uniform:

            break;
        case GLBufferUsage::Storage:
        
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

void OpenGLGraphicsDevice::gl_bind_texture(uint32_t slot, GLTexture& tex) {
    assert(executing);
    assert(&tex.device == this);
    if(slot >= max_bound_textures) {
        throw std::runtime_error("OpenGL bind_texture slot should be less than " + std::to_string(slot));
    }

    glBindTextureUnit(slot, tex.handle);
}

void OpenGLGraphicsDevice::gl_bind_buffer(uint32_t slot, GLBufferUsage usage, GLBuffer& buf) {
    assert(executing);
    assert(&buf.device == this);
    if(slot >= max_bound_buffers) {
        throw std::runtime_error("OpenGL bind_buffer slot should be less than " + std::to_string(slot));
    }

    switch(usage) {
    case GLBufferUsage::Vertex:
        
        break;
    case GLBufferUsage::Index:

        break;
    case GLBufferUsage::Uniform:

        break;
    case GLBufferUsage::Storage:
    
        break;
    default:
        assert(false);
    }
}

void OpenGLGraphicsDevice::gl_bind_framebuffer(GLFramebuffer& tgt) {
    assert(executing);
    assert(&tgt.device == this);

    GLuint gl_handle = tgt.handle;
    glBindFramebuffer(GL_FRAMEBUFFER, gl_handle);
}

void OpenGLGraphicsDevice::gl_bind_graphics_pipeline(GLGraphicsProgram& pipeline) {
    assert(executing);
    assert(&pipeline.device == this);

    // TODO
}

void OpenGLGraphicsDevice::gl_bind_compute_pipeline(GLComputeProgram& pipeline) {
    assert(executing);
    assert(&pipeline.device == this);

    // TODO
}

void OpenGLGraphicsDevice::gl_set_viewport(
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

void OpenGLGraphicsDevice::gl_set_scissor(
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

void OpenGLGraphicsDevice::gl_draw(uint32_t count, uint32_t offset) {
    assert(executing);

    // TODO
}

void OpenGLGraphicsDevice::gl_dispatch(uint32_t x, uint32_t y, uint32_t z) {
    assert(executing);
    
    // TODO
}


// -- PIPELINE --
std::string load_file(const fs::path& filepath) {
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if(!file) {
        throw std::runtime_error("Cannot open file : " + filepath.string());
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::unique_ptr<engine::pipeline::Pipeline> OpenGLGraphicsDevice::create_pipeline() {
    return std::make_unique<engine::pipeline::opengl::Pipeline>(*this);
}

std::unique_ptr<engine::pipeline::Program> OpenGLGraphicsDevice::create_program(std::vector<engine::pipeline::Shader*> shaders) {
    // TODO
    // need to infer what sort of program to make based off of the given shaders
}

std::unique_ptr<engine::pipeline::Shader> OpenGLGraphicsDevice::create_shader(fs::path source_path) {
    // load source code
    std::string source_code = load_file(source_path);
    
    // determine what sort of shader this is via extension
    GLShaderType gl_shader_type;
    shaderc_shader_kind sc_kind;
    if(source_path.extension() == ".vert") {
        gl_shader_type = GLShaderType::Vertex;
        sc_kind = shaderc_glsl_vertex_shader;
    } 
    else if(source_path.extension() == ".frag") {
        gl_shader_type = GLShaderType::Fragment;
        sc_kind = shaderc_glsl_fragment_shader;
    } 
    else if(source_path.extension() == ".compute") {
        gl_shader_type = GLShaderType::Compute;
        sc_kind = shaderc_glsl_compute_shader;
    } 
    else {
        throw std::runtime_error("Unknown shader extension: " + source_path.extension().string());
    }

    // create a compiler
    shaderc_compiler_t sc_compiler = shaderc_compiler_initialize();
    if(!sc_compiler) {
        throw std::runtime_error("Failed to initialize shaderc");
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
        throw std::runtime_error("Shader compilation failed : \n" + error);
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
    GLShader gl_shader = gl_create_shader(gl_shader_type, source_code);
    std::unique_ptr<engine::pipeline::Shader> shader = std::unique_ptr<engine::pipeline::opengl::Shader>(
        new engine::pipeline::opengl::Shader(
            source_code, spirv_code,
            *this, std::move(gl_shader)
        )
    );
    return shader;
}

}
