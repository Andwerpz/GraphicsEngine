#pragma once
#include <cstdint>

namespace engine::graphics {

class Window {
public:
    Window() = default;

    // make Window non-moveable to make it have a stable address once allocated
    Window(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;

    virtual uint32_t get_width() = 0;
    virtual uint32_t get_height() = 0;
};

}