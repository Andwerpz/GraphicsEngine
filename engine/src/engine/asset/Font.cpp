#include <engine/asset/Font.h>
#include <engine/utils/error.h>
#include <cassert>
#include <optional>

#include <freetype/freetype.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>

namespace engine::asset {

void print_bitmap(const FT_Bitmap& bitmap) {
    constexpr char ramp[] = " .:-=+*#%@";
    constexpr int ramp_size = sizeof(ramp) - 1;
    for(unsigned int y = 0; y < bitmap.rows; ++y) {
        const unsigned char* row;
        if(bitmap.pitch >= 0) {
            row = bitmap.buffer + y * bitmap.pitch;
        }
        else {
            row = bitmap.buffer + (bitmap.rows - 1 - y) * (-bitmap.pitch);
        }
        for(unsigned int x = 0; x < bitmap.width; ++x) {
            unsigned char pixel = row[x];
            int index = pixel * (ramp_size - 1) / 255;
            std::cout << ramp[index] << ramp[index];
        }
        std::cout << '\n';
    }
}

struct Pixel {
    float r, g, b, a;
};

// returns array : pixels[x][y]
std::vector<std::vector<Pixel>> decode_bitmap(const FT_Bitmap& bitmap) {
    std::vector<std::vector<Pixel>> pixels(bitmap.width, std::vector<Pixel>(bitmap.rows));
    auto grayscale_pixel = [](float coverage) -> Pixel {
        return {1.0f, 1.0f, 1.0f, coverage};
    };

    for(uint32_t y = 0; y < bitmap.rows; y++) {
        const uint8_t* row;
        if(bitmap.pitch >= 0)
            row = bitmap.buffer + y * bitmap.pitch;
        else
            row = bitmap.buffer + (bitmap.rows - 1 - y) * (-bitmap.pitch);

        for(uint32_t x = 0; x < bitmap.width; x++) {
            Pixel px{0.0f, 0.0f, 0.0f, 0.0f};
            switch(bitmap.pixel_mode) {
                case FT_PIXEL_MODE_NONE:
                    // no pixel data
                    break;
                case FT_PIXEL_MODE_MONO: {
                    // 1 bit per pixel, most significant bit first
                    float coverage = (row[x / 8] & (0x80 >> (x % 8))) ? 1.0f : 0.0f;
                    px = grayscale_pixel(coverage);
                    break;
                }
                case FT_PIXEL_MODE_GRAY: {
                    // 8-bit grayscale
                    float coverage = static_cast<float>(row[x]) / static_cast<float>(bitmap.num_grays - 1);
                    px = grayscale_pixel(coverage);
                    break;
                }
                case FT_PIXEL_MODE_GRAY2: {
                    // 2 bits per pixel
                    uint8_t shift = 6 - 2 * (x % 4);
                    uint8_t value = (row[x / 4] >> shift) & 0x03;
                    px = grayscale_pixel(value / 3.0f);
                    break;
                }
                case FT_PIXEL_MODE_GRAY4: {
                    // 4 bits per pixel
                    uint8_t shift = 4 - 4 * (x % 2);
                    uint8_t value = (row[x / 2] >> shift) & 0x0F;
                    px = grayscale_pixel(value / 15.0f);
                    break;
                }
                case FT_PIXEL_MODE_LCD:
                case FT_PIXEL_MODE_LCD_V:
                    // subpixel rendering for LCD screens
                    engine::utils::throw_error("FT_PIXEL_MODE_LCD/LCD_V is not supported");
                case FT_PIXEL_MODE_BGRA: {
                    // 32-bit premultiplied BGRA
                    const uint8_t* p = row + x * 4;
                    px = {
                        p[2] / 255.0f,
                        p[1] / 255.0f,
                        p[0] / 255.0f,
                        p[3] / 255.0f
                    };
                    break;
                }
                default:
                    engine::utils::throw_error("Unknown FreeType pixel mode");
            }
            pixels[x][y] = px;
        }
    }
    return pixels;
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

    // load glyph dimensions
    std::map<uint64_t, Font::Glyph> glyphs;
    std::map<uint64_t, std::vector<std::vector<Pixel>>> bitmaps;
    {
        FT_UInt glyph_index;
        FT_ULong char_code = FT_Get_First_Char(face, &glyph_index);
        while(glyph_index != 0) {
            if(FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER) == 0) {
                FT_GlyphSlot ft_glyph = face->glyph;
                if(char_code >= 128) {
                    // non ascii char, skip for now
                }
                else {
                    assert(!glyphs.contains(char_code));

                    Font::Glyph glyph;
                    glyph.char_code = char_code;
                    glyph.width_px = ft_glyph->bitmap.width;
                    glyph.height_px = ft_glyph->bitmap.rows;
                    glyph.advance_x_px = ft_glyph->advance.x / 64.0f;
                    glyph.bearing_x_px = ft_glyph->bitmap_left;
                    glyph.bearing_y_px = -ft_glyph->bitmap_top;
                    glyphs.insert({char_code, glyph});
                    bitmaps.insert({char_code, decode_bitmap(ft_glyph->bitmap)});
                }
            }
            else {
                engine::utils::throw_error("Failed to load glyph : \'" + std::to_string(char_code) + "\' : " + path.string());
            }
            char_code = FT_Get_Next_Char(face, char_code, &glyph_index);
        }
    }
    FT_Done_Face(face);
    FT_Done_FreeType(library);

    // pack glyphs into atlas
    // for now, we're not doing anything smart, just doing shelf packing
    auto build_atlas = [&glyphs, &bitmaps](
        uint32_t& atlas_width,
        uint32_t& atlas_height,
        Image* atlas = nullptr
    ) -> void {
        // minimum number of pixels between any two adjacent glyphs and texture border
        constexpr uint32_t PADDING_PX = 1;

        // fixed atlas width
        atlas_width = 256;
        atlas_height = 0;

        // pack shelves, figure out atlas height
        uint32_t xptr = PADDING_PX, yptr = PADDING_PX;
        for(auto i = glyphs.begin(); i != glyphs.end(); i++) {
            uint64_t char_code = i->first;
            Font::Glyph& glyph = i->second;
            assert(bitmaps.contains(char_code));
            std::vector<std::vector<Pixel>>& bitmap = bitmaps.at(char_code);
            uint32_t width = glyph.width_px;
            uint32_t height = glyph.height_px;

            // see if we don't need to place
            if(width == 0 || height == 0) continue;

            // see if we need to move to the next row
            assert(width + PADDING_PX * 2 <= atlas_width);
            if(xptr + width + PADDING_PX > atlas_width) {
                yptr = atlas_height + PADDING_PX;
                xptr = PADDING_PX;
            }

            // place glyph
            glyph.atlas_x_px = xptr;
            glyph.atlas_y_px = yptr;
            atlas_height = std::max(atlas_height, yptr + height);
            xptr += width + PADDING_PX;
            assert(xptr <= atlas_width);

            // fill pixels
            if(atlas != nullptr) {
                assert(atlas->get_width() == atlas_width);
                assert(atlas->get_height() >= atlas_height);
                for(uint32_t xoff = 0; xoff < width; xoff++) {
                    for(uint32_t yoff = 0; yoff < height; yoff++) {
                        Pixel& px = bitmap[xoff][yoff];
                        uint32_t x = glyph.atlas_x_px + xoff;
                        uint32_t y = glyph.atlas_y_px + yoff;
                        atlas->set_pixel(
                            x, y,
                            px.r, px.g, px.b, px.a
                        );
                    }
                }
            }
        }

        // case where no glyph has a bitmap
        if(atlas_height == 0) {
            atlas_width = 1;
            atlas_height = 1;
        }

        // sanity check
        if(atlas != nullptr) {
            assert(atlas->get_width() == atlas_width);
            assert(atlas->get_height() == atlas_height);
        }
    };

    // determine atlas dimensions
    uint32_t atlas_width, atlas_height;
    build_atlas(atlas_width, atlas_height);

    // populate atlas pixels 
    Image atlas(
        atlas_width,
        atlas_height,
        ImageFormat::RGBA8
    );
    build_atlas(atlas_width, atlas_height, &atlas);

    return Font(
        std::move(atlas),
        std::move(glyphs)
    );
}

Font::Font(Font&& other) :
    atlas(std::move(other.atlas)),
    glyphs(std::move(other.glyphs))
{
    // do nothing
}

Font::Font(
    Image&& atlas,
    std::map<uint64_t, Glyph>&& glyphs
) : 
    atlas(std::move(atlas)),
    glyphs(std::move(glyphs))
{
    // do nothing
}

Font::Glyph& Font::get_glyph(uint64_t char_code) {
    if(!glyphs.contains(char_code)) {
        engine::utils::throw_error("no glyph with char code : " + std::to_string(char_code));
    }
    return glyphs.at(char_code);
}

}