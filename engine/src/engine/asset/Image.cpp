#include <engine/asset/Image.h>
#include <engine/util/error.h>
#include <engine/util/file.h>
#include <engine/asset/EmbeddedResource.h>

#include <cassert>
#include <memory>
#include <cstring>
#include <iostream>

#include <stb_image.h>
#include <stb_image_write.h>

namespace engine::asset {

static bool format_has_alpha(ImageFormat format) {
    switch(format) {
        case ImageFormat::RGBA8:
            return true;
        default:
            assert(false);
    }
}

Image::Image(Image&& other) :
    Asset(std::move(other))
{
    width = other.width;
    height = other.height;
    format = other.format;
    alpha_mode = other.alpha_mode;
    data = std::move(other.data);

    uint64_t data_size = get_pixel_size() * width * height;
    assert(data.size() == data_size);
}

Image::Image(
    uint32_t _width, 
    uint32_t _height,
    ImageFormat _format,
    ImageAlphaMode _alpha_mode
) : Asset() {
    if(format_has_alpha(_format) && _alpha_mode == ImageAlphaMode::None) {
        engine::util::throw_error("Image format has alpha channel, alpha mode cannot be None");
    }
    if(!format_has_alpha(_format) && _alpha_mode != ImageAlphaMode::None) {
        engine::util::throw_error("Image format does not have alpha channel, alpha mode must be None");
    }

    width = _width;
    height = _height;
    format = _format;
    alpha_mode = _alpha_mode;
    
    uint64_t data_size = get_pixel_size() * width * height;
    data = std::vector<std::byte>((size_t) data_size, (std::byte) 0);
}

Image::Image(
    std::string_view addr,
    uint32_t _width,
    uint32_t _height,
    ImageFormat _format,
    ImageAlphaMode _alpha_mode,
    std::vector<std::byte>&& _data
) : Asset(addr) {
    if(format_has_alpha(_format) && _alpha_mode == ImageAlphaMode::None) {
        engine::util::throw_error("Image format has alpha channel, alpha mode cannot be None");
    }
    if(!format_has_alpha(_format) && _alpha_mode != ImageAlphaMode::None) {
        engine::util::throw_error("Image format does not have alpha channel, alpha mode must be None");
    }

    width = _width;
    height = _height;
    format = _format;
    alpha_mode = _alpha_mode;
    data = std::move(_data);
}

Image Image::load(std::string_view addr) {
    std::vector<std::byte> bytes = load_asset(addr);
    if(bytes.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
        engine::util::throw_error("Image data too large");
    }

    // stbi_load is one byte per channel interface
    // use stbi_loadf for one float per channel
    int width, height, channels;
    unsigned char* pixels = stbi_load_from_memory(
        reinterpret_cast<const stbi_uc*>(bytes.data()),
        static_cast<int>(bytes.size()),
        &width, &height, &channels,
        4   // force RGBA
    );
    if(!pixels) {
        engine::util::throw_error(std::string("Failed to decode image : ") + stbi_failure_reason());
    }

    // copy pixel data
    const size_t data_size = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
    std::vector<std::byte> data(data_size);
    std::memcpy(data.data(), pixels, data_size);
    stbi_image_free(pixels);

    return Image(
        addr,
        width, height, 
        ImageFormat::RGBA8, 
        ImageAlphaMode::Straight, 
        std::move(data)
    );
}

void Image::export_as_png(const fs::path& path) {
    std::string filename = path.string();
    switch(format) {
        case ImageFormat::RGBA8: {
            // un-premultiply if necessary
            const void* out_data = get_data();
            std::vector<uint8_t> straight;
            if(alpha_mode == ImageAlphaMode::Premultiplied) {
                uint64_t pixel_count = (uint64_t) width * (uint64_t) height;
                const uint8_t* src = reinterpret_cast<const uint8_t*>(get_data());
                straight.resize(pixel_count * 4);

                for(uint64_t i = 0; i < pixel_count; i++) {
                    uint32_t a = src[i * 4 + 3];
                    for(int c = 0; c < 3; c++) {
                        uint32_t out = a ? (src[i * 4 + c] * 255 + a / 2) / a : 0;
                        straight[i * 4 + c] = static_cast<uint8_t>(out > 255 ? 255 : out);
                    }
                    straight[i * 4 + 3] = static_cast<uint8_t>(a);
                }
                out_data = straight.data();
            }

            // write png
            if(!stbi_write_png(
                filename.c_str(),
                width,
                height,
                4,              // RGBA = 4 channels
                out_data,       // pointer to pixel data
                width * 4       // stride in bytes per row
            )) {
                engine::util::throw_error("Failed to write PNG : " + filename);
            }
            break;
        }
        default:
            engine::util::throw_error("unsupported image format for PNG export");
    }
}

uint64_t Image::get_pixel_size() const {
    switch(format) {
        case ImageFormat::RGBA8:
            return 4;
        default:
            assert(false);
    }
}

glm::vec4 Image::get_pixel(uint32_t x, uint32_t y) const {
    if(x >= width || y >= height) {
        engine::util::throw_error("pixel coordinates out of range");
    }

    uint64_t pixel_size = get_pixel_size();
    switch(format) {
        case ImageFormat::RGBA8: {
            uint64_t off = pixel_size * (width * y + x);
            auto to_float = [](std::byte b) -> float {
                return (float) b / 255.0f;
            };
            return {
                to_float(data[off + 0]),
                to_float(data[off + 1]),
                to_float(data[off + 2]),
                to_float(data[off + 3])
            };
        }
        default:
            assert(false);
    }
}

void Image::set_pixel(uint32_t x, uint32_t y, glm::vec4 color) {
    if(x >= width || y >= height) {
        engine::util::throw_error("pixel coordinates out of range");
    }

    uint64_t pixel_size = get_pixel_size();
    switch(format) {
        case ImageFormat::RGBA8: {
            uint64_t off = pixel_size * (width * y + x);
            auto to_byte = [](float c) -> std::byte {
                c = std::max(0.0f, std::min(1.0f, c));
                return std::byte(static_cast<uint8_t>(c * 255.0f + 0.5f));
            };
            data[off + 0] = to_byte(color.r);
            data[off + 1] = to_byte(color.g);
            data[off + 2] = to_byte(color.b);
            data[off + 3] = to_byte(color.a);
            break;
        }
        default:
            assert(false);
    }
}

}