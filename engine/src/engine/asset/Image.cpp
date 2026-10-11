#include <engine/asset/Image.h>
#include <engine/utils/error.h>

#include <cassert>
#include <memory>
#include <cstring>

#include <stb_image.h>
#include <stb_image_write.h>

namespace engine::asset {

Image::Image(Image&& other) {
    width = other.width;
    height = other.height;
    format = other.format;
    data = std::move(other.data);

    uint64_t data_size = get_pixel_size() * width * height;
    assert(data.size() == data_size);
}

Image::Image(
    uint32_t _width, 
    uint32_t _height,
    ImageFormat _format
) {
    width = _width;
    height = _height;
    format = _format;
    
    uint64_t data_size = get_pixel_size() * width * height;
    data = std::vector<std::byte>((size_t) data_size, (std::byte) 0);
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
    // make sure this path exists and refers to a regular file
    if(!fs::exists(path)) {
        engine::utils::throw_error("path does not exist : " + path.string());
    }
    if(!fs::is_regular_file(path)) {
        engine::utils::throw_error("path does not refer to regular file : " + path.string());
    }

    // stbi_load is one byte per channel interface
    // use stbi_loadf for one float per channel
    int width;
    int height;
    int channels;
    std::string filename = path.string();
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

void Image::export_as_png(const fs::path& path) {
    std::string filename = path.string();
    switch(format) {
        case ImageFormat::RGBA8:
            stbi_write_png(
                filename.c_str(),
                width,
                height,
                4,                      // RGBA = 4 channels
                get_data(),             // pointer to pixel data
                width * 4               // stride in bytes per row
            );
            break;
        default:
            assert(false);
    }
}

uint64_t Image::get_pixel_size() {
    switch(format) {
        case ImageFormat::RGBA8:
            return 4;
        default:
            assert(false);
    }
}

void Image::set_pixel(
    uint32_t x, uint32_t y,
    float r, float g, float b, float a
) {
    if(x >= width || y >= height) {
        engine::utils::throw_error("coordinates out of range");
    }

    uint64_t pixel_size = get_pixel_size();
    switch(format) {
        case ImageFormat::RGBA8: {
            uint64_t off = pixel_size * (width * y + x);
            auto to_byte = [](float c) -> std::byte {
                c = std::max(0.0f, std::min(1.0f, c));
                return std::byte(static_cast<uint8_t>(c * 255.0f + 0.5f));
            };
            data[off + 0] = to_byte(r);
            data[off + 1] = to_byte(g);
            data[off + 2] = to_byte(b);
            data[off + 3] = to_byte(a);
            break;
        }
        default:
            assert(false);
    }
}

}