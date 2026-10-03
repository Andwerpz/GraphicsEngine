#pragma once
#include <vector>
#include <cstdint>

#include <filesystem>
namespace fs = std::filesystem;

namespace engine::assets {

// 2D grid of pixels
// origin should be the top left (the first pixel in the data)

// format of each pixel
enum class ImageFormat {
    RGBA8,
};

class Image {
public:
    Image(Image&& other);
    ~Image() = default;

    static Image load_image(const fs::path& path);

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

private:
    Image(
        uint32_t width, 
        uint32_t height,
        ImageFormat format,
        std::vector<std::byte>&& data
    );

    // image dimensions
    uint32_t width;
    uint32_t height;

    // data format
    ImageFormat format;

    // opaque byte array
    std::vector<std::byte> data;
};

}