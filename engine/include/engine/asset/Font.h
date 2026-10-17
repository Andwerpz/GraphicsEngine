#pragma once
#include <cstdint>
#include <map>
#include <span>
#include <filesystem>
namespace fs = std::filesystem;

#include "Asset.h"
#include "Image.h"

namespace engine::asset {

class Font : public Asset {
public:
    // identifies rectangular region within atlas
    struct Glyph {
        uint32_t char_code;

        // top left corner of this glyph in the atlas
        // coordinates will be placeholders if width or height is 0 (for example, for a space char)
        uint32_t atlas_x, atlas_y;

        // dimensions of the glyph bitmap
        uint32_t width, height;

        // horizontal distance to advance the pen when rendering
        float advance_x;

        // offset from the pen origin to the bitmap's top left corner
        float bearing_x, bearing_y;
    };

    Font(Font&& other);
    ~Font() = default;

    static Font load_from_memory(std::span<const std::byte> data, float font_size);
    static Font load(std::string_view addr, float font_size);

    Image& get_atlas() {
        return atlas;
    }

    Glyph& get_glyph(uint64_t char_code);

    float get_ascent() {
        return ascent;
    }
    
    float get_descent() {
        return descent;
    }

    float get_line_height() {
        return line_height;
    }

    // returns the width in pixels if the given text were to be rendered in a single line
    float measure_text_width(const std::string_view& text);

private:
    Font(
        Image&& atlas,
        std::map<uint64_t, Glyph>&& glyphs,
        float ascent,
        float descent,
        float line_height
    );

    // image containing all loaded glyphs
    Image atlas;

    // mapping from char_code -> glyph
    std::map<uint64_t, Glyph> glyphs;

    // distance the font extends above the baseline
    float ascent;

    // distance the font extends below the baseline
    float descent;

    // recommended baseline to baseline spacing
    float line_height;
};

}