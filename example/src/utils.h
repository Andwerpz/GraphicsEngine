#pragma once
#include <engine/graphics/opengl/GLGraphicsDevice.h>
#include <engine/graphics/GraphicsDevice.h>
#include <engine/graphics/Shader.h>
#include <engine/graphics/Buffer.h>
#include <engine/asset/Image.h>

#include <engine/graphics/opengl/GLBuffer.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <filesystem>
namespace fs = std::filesystem;

#include <iostream>
#include <chrono>
#include <thread>

inline std::unique_ptr<engine::graphics::Shader> create_shader(engine::graphics::GraphicsDevice& device, fs::path path) {
    if(!fs::exists(path)) {
        throw std::runtime_error("shader doesn't exist : " + path.string());
    }
    return device.create_shader(path);
}

inline std::unique_ptr<engine::graphics::RenderingProgram> create_rendering_program(
    engine::graphics::GraphicsDevice& device, 
    fs::path vertex_path, 
    fs::path fragment_path,
    engine::graphics::PrimitiveTopology primitive_topology
) {
    auto vertex_shader = create_shader(device, vertex_path);
    auto fragment_shader = create_shader(device, fragment_path);
    return device.create_rendering_program(
        *vertex_shader, 
        *fragment_shader, 
        primitive_topology
    );
}

inline constexpr float cube_vertices[] = {
    // Front (+Z)
    -0.5f, -0.5f,  0.5f,
    0.5f, -0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

    // Back (-Z)
    0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,

    0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,

    // Left (-X)
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,

    // Right (+X)
    0.5f, -0.5f,  0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,

    0.5f, -0.5f,  0.5f,
    0.5f,  0.5f, -0.5f,
    0.5f,  0.5f,  0.5f,

    // Top (+Y)
    -0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    0.5f,  0.5f, -0.5f,

    -0.5f,  0.5f,  0.5f,
    0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,

    // Bottom (-Y)
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
};

inline constexpr float cube_normals[] = {
    // Front (+Z)
     0,  0,  1,
     0,  0,  1,
     0,  0,  1,
     0,  0,  1,
     0,  0,  1,
     0,  0,  1,

    // Back (-Z)
     0,  0, -1,
     0,  0, -1,
     0,  0, -1,
     0,  0, -1,
     0,  0, -1,
     0,  0, -1,

    // Left (-X)
    -1,  0,  0,
    -1,  0,  0,
    -1,  0,  0,
    -1,  0,  0,
    -1,  0,  0,
    -1,  0,  0,

    // Right (+X)
     1,  0,  0,
     1,  0,  0,
     1,  0,  0,
     1,  0,  0,
     1,  0,  0,
     1,  0,  0,

    // Top (+Y)
     0,  1,  0,
     0,  1,  0,
     0,  1,  0,
     0,  1,  0,
     0,  1,  0,
     0,  1,  0,

    // Bottom (-Y)
     0, -1,  0,
     0, -1,  0,
     0, -1,  0,
     0, -1,  0,
     0, -1,  0,
     0, -1,  0,
};

inline constexpr float cube_uvs[] = {
    // Front (+Z)
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    // Back (-Z)
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    // Left (-X)
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    // Right (+X)
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    // Top (+Y)
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    // Bottom (-Y)
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
};