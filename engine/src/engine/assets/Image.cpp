#include <engine/assets/Image.h>
#include <engine/utils/error.h>

#include <memory>
#include <cstring>

#include <stb_image.h>

namespace engine::assets {

Image::Image(Image&& other) {
    width = other.width;
    height = other.height;
    format = other.format;
    data = std::move(other.data);
}

Image::Image(
    uint32_t _width,
    uint32_t _height,
    ImageFormat _format,
    std::vector<std::byte>&& _data
) {
    width = _width;
    height = _height;
    format = _format;
    data = std::move(_data);
}

Image Image::load_image(const fs::path& path) {
    int width;
    int height;
    int channels;
    std::string filename = path.string();

    // stbi_load is one byte per channel interface
    // use stbi_loadf for one float per channel
    unsigned char* pixels = stbi_load(
        filename.c_str(),
        &width,
        &height,
        &channels,
        4       // force RGBA
    );
    if(!pixels) {
        engine::utils::throw_error("Failed to load image : \"" + filename + "\" : " + stbi_failure_reason());
    }

    uint64_t data_size = (uint64_t) width * (uint64_t) height * 4;
    std::vector<std::byte> data(data_size);
    std::memcpy(data.data(), pixels, data_size);
    Image image(
        width,
        height,
        ImageFormat::RGBA8,
        std::move(data)
    );

    stbi_image_free(pixels);
    return image;
}
}