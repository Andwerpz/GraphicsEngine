#pragma once
#include <glm/glm.hpp>

#include <engine/asset/Image.h>
#include <engine/math/Rect.h>

namespace engine::editor {

// an ImageEditor holds a private mutable Image that it can modify freely
// you can access this image via get_image() which makes a copy of it
class ImageEditor {
    using Image = engine::asset::Image;
    using ImageAlphaMode = engine::asset::ImageAlphaMode;
    using ImageFormat = engine::asset::ImageFormat;

public:
    explicit ImageEditor(const Image& source);

    uint32_t get_width() const { return editable.get_width(); }
    uint32_t get_height() const { return editable.get_height(); }
    ImageFormat get_format() const { return editable.get_format(); }
    ImageAlphaMode get_alpha_mode() const { return editable.get_alpha_mode(); }

    // copy of the editable image
    Image get_image() const { 
        return editable;
    }

    // true after any pixel of the editable image has been written
    bool is_dirty() const {
        return dirty;
    }

    // retrieves the stored pixel
    // the pixel with be in the alpha mode of the underlying image
    glm::vec4 get_pixel(uint32_t x, uint32_t y) const;

    // sets the pixel color
    // expects color to be straight, this converts to the proper alpha mode
    void set_pixel(uint32_t x, uint32_t y, glm::vec4 color);

    // sets the color of all pixels in the rect, clipped to the image
    // returns true when a pixel is contained by rect
    bool fill_rect(engine::math::Rect2i rect, glm::vec4 color);

private:
    // converts a straight alpha color into an equivalent color of the underlying alpha mode
    glm::vec4 to_stored(glm::vec4 color) const;

    Image editable;
    bool dirty = false;
};

}
