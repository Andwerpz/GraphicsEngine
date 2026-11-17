#pragma once
#include <memory>
#include <cstdint>
#include <set>
#include <vector>
#include <glm/glm.hpp>
#include <engine/graphics/GraphicsDevice.h>
#include <engine/graphics/Texture.h>
#include <engine/math/Rect.h>

// a CubeSet represents axis aligned unit cubes on a grid
// it is capable of rasterizing a position texture of these unit cubes using an orthographic view

// a cube at position (x, y, z) fills the volume [x, x + 1] x [y, y + 1] x [z, z + 1]
// when rendering, the camera is angled at 45 deg facing towards the -y / -z axes
//   it is positioned so that the origin is at the top left corner of the viewport
//   and it uses an orthographic projection

// PositionEncoding::Raw stores the position in world units
// PositionEncoding::Normalized normalizes all the positions into [0, 1]
// texels with no cube in them should have color (0, 0, 0, 0)

class CubeSet {
    using GraphicsDevice = engine::graphics::GraphicsDevice;
    using Texture = engine::graphics::Texture;

    struct CellLess {
        bool operator()(const glm::ivec3& a, const glm::ivec3& b) const {
            if(a.x != b.x) return a.x < b.x;
            if(a.y != b.y) return a.y < b.y;
            return a.z < b.z;
        }
    };

public:
    enum class PositionEncoding {
        Raw,
        Normalized,
    };

    void insert(glm::ivec3 cube);
    bool contains(glm::ivec3 cube) const;

    // minimum corner of every cube
    std::vector<glm::ivec3> get_cubes() const;

    // partitions the cubes contained by this set into a collection of 3D rectangles
    // this tries to return the minimum amount of rectangles possible
    std::vector<engine::math::Rect3i> get_boxes() const;

    std::unique_ptr<Texture> create_texture(
        GraphicsDevice& device,
        uint32_t view_width,
        uint32_t view_height,
        uint32_t cube_size,
        PositionEncoding encoding = PositionEncoding::Raw
    ) const;

private:
    std::set<glm::ivec3, CellLess> cubes;

    std::vector<glm::vec4> raster_positions(
        uint32_t view_width,
        uint32_t view_height,
        uint32_t cube_size
    ) const;
};
