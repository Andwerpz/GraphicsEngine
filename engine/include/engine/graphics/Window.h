#pragma once
#include <cstdint>

namespace engine::graphics {

class Window {
public:
    virtual uint32_t get_width() = 0;
    virtual uint32_t get_height() = 0;
};

}