#include <engine/editor/ImageEditor.h>
#include <engine/util/error.h>

#include <algorithm>
#include <cassert>
#include <cstddef>

namespace engine::editor {

ImageEditor::ImageEditor(const Image& source) : editable(source) {}

glm::vec4 ImageEditor::get_pixel(uint32_t x, uint32_t y) const {
    return editable.get_pixel(x, y);
}

void ImageEditor::set_pixel(uint32_t x, uint32_t y, glm::vec4 color) {
    editable.set_pixel(x, y, to_stored(color));
    dirty = true;
}

bool ImageEditor::fill_rect(engine::math::Rect2i rect, glm::vec4 color) {
    // intersect fill rect with image bounds
    engine::math::Rect2i bounds{{0, 0}, {(int) get_width(), (int) get_height()}};
    rect = rect.intersect(bounds);
    if(rect.is_empty()) {
        return false;
    }

    // fill color
    int x0 = rect.pos.x;
    int y0 = rect.pos.y;
    int x1 = rect.max().x;
    int y1 = rect.max().y;
    color = to_stored(color);
    auto to_byte = [](float c) -> std::byte {
        c = std::max(0.0f, std::min(1.0f, c));
        return std::byte(static_cast<uint8_t>(c * 255.0f + 0.5f));
    };
    switch(get_format()) {
        case engine::asset::ImageFormat::RGBA8: {
            std::byte px[4] = {
                to_byte(color.r),
                to_byte(color.g),
                to_byte(color.b),
                to_byte(color.a)
            };
            uint32_t width = get_width();
            std::byte* data = editable.data.data();
            for(int y = y0; y < y1; y++) {
                std::byte* row = data + ((uint64_t) width * (uint64_t) y + (uint64_t) x0) * 4;
                for(int x = x0; x < x1; x++) {
                    row[0] = px[0];
                    row[1] = px[1];
                    row[2] = px[2];
                    row[3] = px[3];
                    row += 4;
                }
            }
            break;
        }
        default:
            assert(false);
            engine::util::throw_error("unsupported image format");
    }
    dirty = true;
    return true;
}

glm::vec4 ImageEditor::to_stored(glm::vec4 color) const {
    switch(get_alpha_mode()) {
        case ImageAlphaMode::None:
        case ImageAlphaMode::Straight:
            // do nothing
            break;
        case ImageAlphaMode::Premultiplied:
            color.r *= color.a;
            color.g *= color.a;
            color.b *= color.a;
            break;
        default:
            engine::util::throw_error("unrecognized image alpha mode in edited image");
    }
    return color;
}

}
