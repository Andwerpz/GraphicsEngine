#include "CubeSet.h"

#include <algorithm>
#include <vector>
#include <engine/util/error.h>

void CubeSet::insert(glm::ivec3 cube) {
    cubes.insert(cube);
}

bool CubeSet::contains(glm::ivec3 cube) const {
    return cubes.contains(cube);
}

std::vector<glm::ivec3> CubeSet::get_cubes() const {
    return std::vector<glm::ivec3>(cubes.begin(), cubes.end());
}

std::vector<engine::math::Rect3i> CubeSet::get_boxes() const {
    auto remaining = cubes;
    std::vector<engine::math::Rect3i> boxes;

    // [min, max) is your current box, we are trying to extend the box max along axis by 1 cell
    // this returns true if we can extend the box (all the new box cells are in remaining)
    auto slice_filled = [&](glm::ivec3 min, glm::ivec3 max, int axis) {
        int at = max[axis];
        glm::ivec3 lo = min;
        glm::ivec3 hi = max;
        lo[axis] = at;
        hi[axis] = at + 1;
        for(int x = lo.x; x < hi.x; x++) {
            for(int y = lo.y; y < hi.y; y++) {
                for(int z = lo.z; z < hi.z; z++) {
                    if(!remaining.contains(glm::ivec3(x, y, z))) return false;
                }
            }
        }
        return true;
    };

    // create boxes until we run out of cells
    while(!remaining.empty()) {
        // start current box at cell, greedily expand it along all axes until you can't anymore
        glm::ivec3 min = *remaining.begin();
        glm::ivec3 max = min + glm::ivec3(1);
        while(slice_filled(min, max, 0)) max.x++;
        while(slice_filled(min, max, 1)) max.y++;
        while(slice_filled(min, max, 2)) max.z++;
        for(int x = min.x; x < max.x; x++) {
            for(int y = min.y; y < max.y; y++) {
                for(int z = min.z; z < max.z; z++) {
                    remaining.erase(glm::ivec3(x, y, z));
                }
            }
        }
        boxes.push_back(engine::math::Rect3i::from_min_max(min, max));
    }
    return boxes;
}

std::vector<glm::vec4> CubeSet::raster_positions(
    uint32_t view_width,
    uint32_t view_height,
    uint32_t cube_size
) const {
    uint64_t tex_width = (uint64_t) view_width * (uint64_t) cube_size;
    uint64_t tex_height = (uint64_t) view_height * (uint64_t) cube_size;
    if(tex_width > UINT32_MAX || tex_height > UINT32_MAX) {
        engine::util::throw_error("texture dimensions overflow");
    }

    // determine for each 'tile', what cube face is going to be rastered
    struct FaceInfo {
        bool occupied = false;
        glm::ivec3 cube_pos{0};
        bool is_front = false;

        // (y + z), larger depth is closer to the camera
        // observe that the depth of two cubes that contend for the same (x, z) tile on the texture
        //   should never be the same
        int depth = 0;       
    };
    int width = (int) view_width;
    int height = (int) view_height;
    std::vector<std::vector<FaceInfo>> to_raster(view_width, std::vector<FaceInfo>(view_height));

    // if there is no cube at (x, z), or the cube already at (x, z) is further away from the camera
    // than the provided cube, sets the cube at (x, z) to the provided cube
    auto consider = [&](int x, int z, const glm::ivec3& cube, bool is_front) {
        if(x < 0 || z < 0 || x >= width || z >= height) return;
        int depth = cube.y + cube.z;
        FaceInfo& slot = to_raster[x][z];
        if(slot.occupied && slot.depth > depth) return;
        slot.occupied = true;
        slot.cube_pos = cube;
        slot.is_front = is_front;
        slot.depth = depth;
    };
    for(const glm::ivec3& cube : cubes) {
        int x = cube.x;
        int z = cube.z - cube.y;
        consider(x, z - 1, cube, false);
        consider(x, z, cube, true);
    }

    // raster positions
    std::vector<glm::vec4> positions(tex_width * tex_height, glm::vec4(0.0f));
    for(int x = 0; x < width; x++) {
        for(int z = 0; z < height; z++) {
            const FaceInfo& face = to_raster[x][z];
            if(!face.occupied) continue;
            for(uint32_t ly = 0; ly < cube_size; ly++) {
                for(uint32_t lx = 0; lx < cube_size; lx++) {
                    float u = ((float) lx + 0.5f) / (float) cube_size;
                    float v = ((float) ly + 0.5f) / (float) cube_size;
                    glm::vec3 pos;
                    if(face.is_front) {
                        pos = glm::vec3(
                            (float) face.cube_pos.x + u,
                            (float) face.cube_pos.y + 1.0f - v,
                            (float) face.cube_pos.z + 1.0f
                        );
                    } 
                    else {
                        pos = glm::vec3(
                            (float) face.cube_pos.x + u,
                            (float) face.cube_pos.y + 1.0f,
                            (float) face.cube_pos.z + v
                        );
                    }
                    uint32_t px = (uint32_t) x * cube_size + lx;
                    uint32_t py = (uint32_t) z * cube_size + ly;
                    positions[(uint64_t) py * tex_width + px] = glm::vec4(pos, 1.0f);
                }
            }
        }
    }

    return positions;
}

std::unique_ptr<CubeSet::Texture> CubeSet::create_texture(
    GraphicsDevice& device,
    uint32_t view_width,
    uint32_t view_height,
    uint32_t cube_size,
    PositionEncoding encoding
) const {
    using namespace engine::graphics;
    if(view_width == 0 || view_height == 0 || cube_size == 0) {
        engine::util::throw_error("view dimensions must be non-zero");
    }

    // raster positions
    std::vector<glm::vec4> positions = raster_positions(view_width, view_height, cube_size);

    // encode positions
    std::vector<float> texels(positions.size() * 4, 0.0f);
    {
        int min_bot = 1e9;
        int max_top = -1e9;
        for(const glm::ivec3& cube : cubes) {
            min_bot = std::min(min_bot, cube.y);
            max_top = std::max(max_top, cube.y + 1);
        }
        float y_extent = (float) (max_top - min_bot);

        for(uint64_t i = 0; i < positions.size(); i++) {
            glm::vec4 encoded = positions[i];
            if(encoding == PositionEncoding::Normalized && encoded.a != 0.0f) {
                encoded = glm::vec4(
                    encoded.x / (float) view_width,
                    (encoded.y - min_bot) / y_extent,
                    encoded.z / (float) view_height,
                    1.0f
                );
            }
            texels[i * 4 + 0] = encoded.r;
            texels[i * 4 + 1] = encoded.g;
            texels[i * 4 + 2] = encoded.b;
            texels[i * 4 + 3] = encoded.a;
        }
    }

    auto texture = device.create_texture(
        TextureDimension::D2,
        TextureFormat::RGBA32F,
        (uint32_t) ((uint64_t) view_width * cube_size),
        (uint32_t) ((uint64_t) view_height * cube_size),
        1, 1
    );
    texture->upload_data(texels.data());
    return texture;
}
