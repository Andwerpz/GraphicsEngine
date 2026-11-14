#pragma once
#include <cstdint>
#include <engine/util/macro.h>

namespace engine::graphics {

class Window {
public:
    Window() = default;

    // make Window non-moveable to make it have a stable address once allocated
    NON_COPYABLE_NON_MOVABLE(Window);

    virtual uint32_t get_width() = 0;
    virtual uint32_t get_height() = 0;
};

}