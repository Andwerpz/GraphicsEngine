#pragma once
#include <cstdint>
#include <map>
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
        // coordinates will be placeholders if width_px or height_px is 0 (for example, for a space char)
        uint32_t atlas_x_px, atlas_y_px;

        // dimensions of the glyph bitmap
        uint32_t width_px, height_px;

        // horizontal distance to advance the pen when rendering
        float advance_x_px;

        // offset from the pen origin to the bitmap's top left corner
        float bearing_x_px, bearing_y_px;
    };

    Font(Font&& other);
    ~Font() = default;

    static Font load_font(const fs::path& path);

    Image& get_atlas() {
        return atlas;
    }

    Glyph& get_glyph(uint64_t char_code);

private:
    Font(
        Image&& atlas,
        std::map<uint64_t, Glyph>&& glyphs
    );

    // image containing all loaded glyphs
    Image atlas;

    // mapping from char_code -> glyph
    std::map<uint64_t, Glyph> glyphs;
};

}