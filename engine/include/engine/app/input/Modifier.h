#pragma once
#include <cstdint>

namespace engine::app {

enum class Modifier : uint8_t {
    None  = 0,
    Shift = 1 << 0,
    Ctrl  = 1 << 1,
    Alt   = 1 << 2,
    Super = 1 << 3,
    CapsLock = 1 << 4,
    NumLock  = 1 << 5
};
constexpr Modifier operator|(Modifier a, Modifier b) {
    return static_cast<Modifier>(
        static_cast<uint8_t>(a) | static_cast<uint8_t>(b)
    );
}
constexpr Modifier operator&(Modifier a, Modifier b) {
    return static_cast<Modifier>(
        static_cast<uint8_t>(a) & static_cast<uint8_t>(b)
    );
}
constexpr bool has_modifier(Modifier modifiers, Modifier modifier) {
    return (modifiers & modifier) != Modifier::None;
}

}