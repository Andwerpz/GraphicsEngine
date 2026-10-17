#include <engine/asset/Font.h>
#include <engine/util/error.h>
#include <engine/util/file.h>
#include <engine/asset/EmbeddedResource.h>
#include <cassert>
#include <optional>

#include <freetype/freetype.h>
#include <freetype/fterrors.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>

namespace engine::asset {

std::string get_ft_error(FT_Error err) {
    const char* msg = FT_Error_String(err);
    std::string text = msg ? msg : "unknown FreeType error";
    return text;
}

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

// returns array : pixels[x][y], pixels are premultiplied alpha
std::vector<std::vector<Pixel>> decode_bitmap(const FT_Bitmap& bitmap) {
    std::vector<std::vector<Pixel>> pixels(bitmap.width, std::vector<Pixel>(bitmap.rows));

    // returns premultiplied grayscale pixel
    auto grayscale_pixel = [](float coverage) -> Pixel {
        return {coverage, coverage, coverage, coverage};
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
                    engine::util::throw_error("FT_PIXEL_MODE_LCD/LCD_V is not supported");
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
                    engine::util::throw_error("Unknown FreeType pixel mode");
            }
            pixels[x][y] = px;
        }
    }
    return pixels;
}

Font Font::load_from_memory(std::span<const std::byte> data, float font_size) {
    // initialize FreeType
    FT_Library library;
    if(FT_Init_FreeType(&library)) {
        engine::util::throw_error("Failed to initialize FreeType");
    }
    FT_Error err;

    // load font
    FT_Face face;
    err = FT_New_Memory_Face(
        library,
        reinterpret_cast<const FT_Byte*>(data.data()),
        static_cast<FT_Long>(data.size_bytes()),
        0,          // face index (non-zero only for collections like .ttc)
        &face
    );
    if(err) {
        engine::util::throw_error("Failed to load font : " + get_ft_error(err));
    }

    // set font size
    err = FT_Set_Pixel_Sizes(face, 0, font_size);
    if(err) {
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        engine::util::throw_error("Failed to set font size : " + get_ft_error(err));
    }

    // load font info
    float ascent = face->size->metrics.ascender / 64.0f;
    float descent = -face->size->metrics.descender / 64.0f;     // FreeType usually stores descent as a negative value
    float line_height = face->size->metrics.height / 64.0f;

    // load glyph dimensions
    std::map<uint64_t, Font::Glyph> glyphs;
    std::map<uint64_t, std::vector<std::vector<Pixel>>> bitmaps;
    {
        FT_UInt glyph_index;
        FT_ULong char_code = FT_Get_First_Char(face, &glyph_index);
        while(glyph_index != 0) {
            err = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
            if(err) {
                engine::util::throw_error("Failed to load glyph : \'" + std::to_string(char_code) + "\' : " + get_ft_error(err));
            }

            FT_GlyphSlot ft_glyph = face->glyph;
            if(char_code >= 128) {
                // non ascii char, skip for now
            }
            else {
                assert(!glyphs.contains(char_code));

                Font::Glyph glyph;
                glyph.char_code = char_code;
                glyph.width = ft_glyph->bitmap.width;
                glyph.height = ft_glyph->bitmap.rows;
                glyph.advance_x = ft_glyph->advance.x / 64.0f;
                glyph.bearing_x = ft_glyph->bitmap_left;
                glyph.bearing_y = -ft_glyph->bitmap_top;
                glyphs.insert({char_code, glyph});
                bitmaps.insert({char_code, decode_bitmap(ft_glyph->bitmap)});
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
            uint32_t width = glyph.width;
            uint32_t height = glyph.height;

            // see if we don't need to place
            if(width == 0 || height == 0) continue;

            // see if we need to move to the next row
            assert(width + PADDING_PX * 2 <= atlas_width);
            if(xptr + width + PADDING_PX > atlas_width) {
                yptr = atlas_height + PADDING_PX;
                xptr = PADDING_PX;
            }

            // place glyph
            glyph.atlas_x = xptr;
            glyph.atlas_y = yptr;
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
                        uint32_t x = glyph.atlas_x + xoff;
                        uint32_t y = glyph.atlas_y + yoff;
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
        ImageFormat::RGBA8,
        ImageAlphaMode::Premultiplied
    );
    build_atlas(atlas_width, atlas_height, &atlas);

    return Font(
        std::move(atlas),
        std::move(glyphs),
        ascent,
        descent,
        line_height
    );
}

Font Font::load(std::string_view addr, float font_size) {
    // load bytes
    std::vector<std::byte> data = load_asset(addr);

    // load font
    return load_from_memory(
        std::span<const std::byte>(data.data(), data.size()),
        font_size
    );
}

Font::Font(Font&& other) :
    atlas(std::move(other.atlas)),
    glyphs(std::move(other.glyphs))
{
    ascent = other.ascent;
    descent = other.descent;
    line_height = other.line_height;
}

Font::Font(
    Image&& atlas,
    std::map<uint64_t, Glyph>&& glyphs,
    float ascent,
    float descent,
    float line_height
) : 
    atlas(std::move(atlas)),
    glyphs(std::move(glyphs)),
    ascent(ascent),
    descent(descent),
    line_height(line_height)
{
    // do nothing
}

Font::Glyph& Font::get_glyph(uint64_t char_code) {
    if(!glyphs.contains(char_code)) {
        engine::util::throw_error("no glyph with char code : " + std::to_string(char_code));
    }
    return glyphs.at(char_code);
}

float Font::measure_text_width(const std::string_view& text) {
    float ans = 0;
    for(int i = 0; i < text.size(); i++) {
        char c = text[i];
        Glyph& glyph = get_glyph(c);
        ans += glyph.advance_x;
    }
    return ans;
}

}