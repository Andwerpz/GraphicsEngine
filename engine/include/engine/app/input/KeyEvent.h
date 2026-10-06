#pragma once
#include <cstdint>

#include "Modifier.h"

namespace engine::app {

enum class Key {
    Unknown,

    // letters
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    // numbers
    Num0, Num1, Num2, Num3, Num4,
    Num5, Num6, Num7, Num8, Num9,

    // punctuation
    Apostrophe,
    Comma,
    Minus,
    Period,
    Slash,
    Semicolon,
    Equal,
    LeftBracket,
    Backslash,
    RightBracket,
    GraveAccent,

    // modifiers
    LeftShift,
    RightShift,
    LeftControl,
    RightControl,
    LeftAlt,
    RightAlt,
    LeftSuper,
    RightSuper,

    // arrow keys
    Left,
    Right,
    Up,
    Down,

    // function keys
    F1, F2, F3, F4, F5, F6,
    F7, F8, F9, F10, F11, F12,

    // keypad
    NumPad0,
    NumPad1,
    NumPad2,
    NumPad3,
    NumPad4,
    NumPad5,
    NumPad6,
    NumPad7,
    NumPad8,
    NumPad9,
    NumPadDecimal,
    NumPadDivide,
    NumPadMultiply,
    NumPadSubtract,
    NumPadAdd,
    NumPadEnter,
    NumPadEqual,

    // navigation
    Escape,
    Tab,
    CapsLock,
    Backspace,
    Enter,
    Space,

    // miscellaneous
    Insert,
    Delete,
    Home,
    End,
    PageUp,
    PageDown,
    PrintScreen,
    ScrollLock,
    Pause,
    Menu
};

enum class KeyAction {
    Press,
    Release,
    Repeat
};

struct KeyEvent {
    Key key;
    KeyAction action;
    Modifier modifier;

    static KeyEvent translate_glfw_key_event(int key, int scancode, int action, int mods);
};

}