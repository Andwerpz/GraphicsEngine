#pragma once
#include <vector>
#include <cstdint>

#include <filesystem>
namespace fs = std::filesystem;

#include "Asset.h"

namespace engine::asset {

// 2D grid of pixels
// origin should be the top left (the first pixel in the data)

// format of each pixel
enum class ImageFormat {
    RGBA8,
};

class Image : public Asset {
public:
    friend class Font;

    Image(Image&& other);
    ~Image() = default;

    static Image load_image(const fs::path& path);

    void export_as_png(const fs::path& path);

    uint32_t get_width() {
        return width;
    }

    uint32_t get_height() {
        return height;
    }

    ImageFormat get_format() {
        return format;
    }

    const std::byte* get_data() {
        return data.data();
    }

    uint64_t get_data_size() {
        return data.size();
    }

    // returns the pixel size in bytes based off of the image format
    uint64_t get_pixel_size();

private:
    // creates an empty image with the given dimensions
    Image(
        uint32_t width, 
        uint32_t height,
        ImageFormat format
    );

    // creates an image with pre-populated data
    Image(
        uint32_t width, 
        uint32_t height,
        ImageFormat format,
        std::vector<std::byte>&& data
    );

    // sets the given pixel to the provided color values
    void set_pixel(
        uint32_t x, uint32_t y,
        float r, float g, float b, float a
    );

    // image dimensions
    uint32_t width;
    uint32_t height;

    // data format
    ImageFormat format;

    // data
    std::vector<std::byte> data;
};

}