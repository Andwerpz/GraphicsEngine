#pragma once
#include <vector>
#include <cstdint>
#include <span>
#include <glm/glm.hpp>

#include <filesystem>
namespace fs = std::filesystem;

#include "Asset.h"

namespace engine::graphics {
class Texture;
}

namespace engine::editor {
class ImageEditor;
}

namespace engine::asset {

// 2D grid of pixels
// origin should be the top left (the first pixel in the data)

// format of each pixel
enum class ImageFormat {
    RGBA8,
};

// how to interpret the color of each pixel
enum class ImageAlphaMode {
    None,           // should be used when no alpha component
    Straight,
    Premultiplied,  // `(r, g, b) / a` will give you the Straight color
};

class Image : public Asset {
public:
    friend class Font;
    friend class engine::editor::ImageEditor;
    friend class engine::graphics::Texture;

    Image(Image&& other);
    ~Image() = default;

    static Image load(std::string_view addr);

    void export_as_png(const fs::path& path);

    uint32_t get_width() const {
        return width;
    }

    uint32_t get_height() const {
        return height;
    }

    ImageFormat get_format() const {
        return format;
    }

    ImageAlphaMode get_alpha_mode() const {
        return alpha_mode;
    }

    uint64_t get_data_size() const {
        return data.size();
    }

    // returns the pixel size in bytes based off of the image format
    uint64_t get_pixel_size() const;

    // gets the given pixel as-is
    glm::vec4 get_pixel(uint32_t x, uint32_t y) const;

private:
    Image(const Image& other);
    Image& operator=(const Image&) = delete;

    // creates an empty image with the given dimensions
    Image(
        uint32_t width, 
        uint32_t height,
        ImageFormat format,
        ImageAlphaMode alpha_mode
    );

    // creates an image with pre-populated data
    Image(
        uint32_t width,
        uint32_t height,
        ImageFormat format,
        ImageAlphaMode alpha_mode,
        std::vector<std::byte>&& data
    );

    // creates an image with pre-populated data from some address
    Image(
        std::string_view addr,
        uint32_t width, 
        uint32_t height,
        ImageFormat format,
        ImageAlphaMode alpha_mode,
        std::vector<std::byte>&& data
    );

    // sets the given pixel to the provided color values
    // does not convert pixel to the image alpha mode, stores the pixel as-is
    void set_pixel(uint32_t x, uint32_t y, glm::vec4 color);

    // gets a pointer to the underlying image data
    const std::byte* get_data() const {
        return data.data();
    }

    // image dimensions
    uint32_t width;
    uint32_t height;

    // data format
    ImageFormat format;

    // pixel interpretation
    ImageAlphaMode alpha_mode;

    // data
    std::vector<std::byte> data;
};

}