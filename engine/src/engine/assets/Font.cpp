#include <engine/assets/Font.h>
#include <engine/utils/error.h>

#include <freetype/freetype.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>

namespace engine::assets {

void print_bitmap(const FT_Bitmap& bitmap) {
    constexpr char ramp[] = " .:-=+*#%@";
    constexpr int ramp_size = sizeof(ramp) - 1;

    for (unsigned int y = 0; y < bitmap.rows; ++y) {
        const unsigned char* row;

        if (bitmap.pitch >= 0) {
            row = bitmap.buffer + y * bitmap.pitch;
        } else {
            row = bitmap.buffer +
                (bitmap.rows - 1 - y) * (-bitmap.pitch);
        }

        for (unsigned int x = 0; x < bitmap.width; ++x) {
            unsigned char pixel = row[x];

            int index = pixel * (ramp_size - 1) / 255;
            std::cout << ramp[index] << ramp[index];
        }

        std::cout << '\n';
    }
}

Font Font::load_font(const fs::path& path) {
    // initialize FreeType
    FT_Library library;
    if(FT_Init_FreeType(&library)) {
        engine::utils::throw_error("Failed to initialize FreeType");
    }

    // load font
    FT_Face face;
    if(FT_New_Face(
        library,
        path.string().c_str(),
        0,
        &face
    )) {
        FT_Done_FreeType(library);
        engine::utils::throw_error("Failed to load font : " + path.string());
    }

    // Request glyphs at 24 pixels per EM
    if (FT_Set_Pixel_Sizes(face, 0, 24)) {
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        engine::utils::throw_error("Failed to set font size : " + path.string());
    }

    for(char c = 0; ; c++) {
        if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            continue;
        }
        std::cout << "Loaded glyph for char : " << (int) c << std::endl;

        FT_GlyphSlot glyph = face->glyph;
        std::cout << "Bitmap size: "
                  << glyph->bitmap.width << " x "
                  << glyph->bitmap.rows << '\n';

        std::cout << "Bearing: "
                  << glyph->bitmap_left << ", "
                  << glyph->bitmap_top << '\n';

        std::cout << "Advance: "
                  << glyph->advance.x / 64.0f << " pixels\n";

        print_bitmap(glyph->bitmap);
        std::cout << "\n";

        // Pixel data:
        // glyph->bitmap.buffer

        if(c == 127) break;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);

    return Font();
}

Font::Font() {
    // TODO
}

}