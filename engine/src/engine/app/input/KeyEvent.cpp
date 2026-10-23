#include <engine/app/input/KeyEvent.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cassert>

#include "Modifier.cpp"

namespace engine::app {

Key translate_glfw_key(int key) {
    switch(key) {
        // letters
        case GLFW_KEY_A: return Key::A;
        case GLFW_KEY_B: return Key::B;
        case GLFW_KEY_C: return Key::C;
        case GLFW_KEY_D: return Key::D;
        case GLFW_KEY_E: return Key::E;
        case GLFW_KEY_F: return Key::F;
        case GLFW_KEY_G: return Key::G;
        case GLFW_KEY_H: return Key::H;
        case GLFW_KEY_I: return Key::I;
        case GLFW_KEY_J: return Key::J;
        case GLFW_KEY_K: return Key::K;
        case GLFW_KEY_L: return Key::L;
        case GLFW_KEY_M: return Key::M;
        case GLFW_KEY_N: return Key::N;
        case GLFW_KEY_O: return Key::O;
        case GLFW_KEY_P: return Key::P;
        case GLFW_KEY_Q: return Key::Q;
        case GLFW_KEY_R: return Key::R;
        case GLFW_KEY_S: return Key::S;
        case GLFW_KEY_T: return Key::T;
        case GLFW_KEY_U: return Key::U;
        case GLFW_KEY_V: return Key::V;
        case GLFW_KEY_W: return Key::W;
        case GLFW_KEY_X: return Key::X;
        case GLFW_KEY_Y: return Key::Y;
        case GLFW_KEY_Z: return Key::Z;

        // numbers
        case GLFW_KEY_0: return Key::Num0;
        case GLFW_KEY_1: return Key::Num1;
        case GLFW_KEY_2: return Key::Num2;
        case GLFW_KEY_3: return Key::Num3;
        case GLFW_KEY_4: return Key::Num4;
        case GLFW_KEY_5: return Key::Num5;
        case GLFW_KEY_6: return Key::Num6;
        case GLFW_KEY_7: return Key::Num7;
        case GLFW_KEY_8: return Key::Num8;
        case GLFW_KEY_9: return Key::Num9;

        // punctuation
        case GLFW_KEY_APOSTROPHE:       return Key::Apostrophe;
        case GLFW_KEY_COMMA:            return Key::Comma;
        case GLFW_KEY_MINUS:            return Key::Minus;
        case GLFW_KEY_PERIOD:           return Key::Period;
        case GLFW_KEY_SLASH:            return Key::Slash;
        case GLFW_KEY_SEMICOLON:        return Key::Semicolon;
        case GLFW_KEY_EQUAL:            return Key::Equal;
        case GLFW_KEY_LEFT_BRACKET:     return Key::LeftBracket;
        case GLFW_KEY_BACKSLASH:        return Key::Backslash;
        case GLFW_KEY_RIGHT_BRACKET:    return Key::RightBracket;
        case GLFW_KEY_GRAVE_ACCENT:     return Key::GraveAccent;

        // modifiers
        case GLFW_KEY_LEFT_SHIFT:    return Key::LeftShift;
        case GLFW_KEY_RIGHT_SHIFT:   return Key::RightShift;
        case GLFW_KEY_LEFT_CONTROL:  return Key::LeftControl;
        case GLFW_KEY_RIGHT_CONTROL: return Key::RightControl;
        case GLFW_KEY_LEFT_ALT:      return Key::LeftAlt;
        case GLFW_KEY_RIGHT_ALT:     return Key::RightAlt;
        case GLFW_KEY_LEFT_SUPER:    return Key::LeftSuper;
        case GLFW_KEY_RIGHT_SUPER:   return Key::RightSuper;

        // arrows
        case GLFW_KEY_LEFT:  return Key::Left;
        case GLFW_KEY_RIGHT: return Key::Right;
        case GLFW_KEY_UP:    return Key::Up;
        case GLFW_KEY_DOWN:  return Key::Down;

        // function keys
        case GLFW_KEY_F1:  return Key::F1;
        case GLFW_KEY_F2:  return Key::F2;
        case GLFW_KEY_F3:  return Key::F3;
        case GLFW_KEY_F4:  return Key::F4;
        case GLFW_KEY_F5:  return Key::F5;
        case GLFW_KEY_F6:  return Key::F6;
        case GLFW_KEY_F7:  return Key::F7;
        case GLFW_KEY_F8:  return Key::F8;
        case GLFW_KEY_F9:  return Key::F9;
        case GLFW_KEY_F10: return Key::F10;
        case GLFW_KEY_F11: return Key::F11;
        case GLFW_KEY_F12: return Key::F12;

        // keypad
        case GLFW_KEY_KP_0:         return Key::NumPad0;
        case GLFW_KEY_KP_1:         return Key::NumPad1;
        case GLFW_KEY_KP_2:         return Key::NumPad2;
        case GLFW_KEY_KP_3:         return Key::NumPad3;
        case GLFW_KEY_KP_4:         return Key::NumPad4;
        case GLFW_KEY_KP_5:         return Key::NumPad5;
        case GLFW_KEY_KP_6:         return Key::NumPad6;
        case GLFW_KEY_KP_7:         return Key::NumPad7;
        case GLFW_KEY_KP_8:         return Key::NumPad8;
        case GLFW_KEY_KP_9:         return Key::NumPad9;
        case GLFW_KEY_KP_DECIMAL:   return Key::NumPadDecimal;
        case GLFW_KEY_KP_DIVIDE:    return Key::NumPadDivide;
        case GLFW_KEY_KP_MULTIPLY:  return Key::NumPadMultiply;
        case GLFW_KEY_KP_SUBTRACT:  return Key::NumPadSubtract;
        case GLFW_KEY_KP_ADD:       return Key::NumPadAdd;
        case GLFW_KEY_KP_ENTER:     return Key::NumPadEnter;
        case GLFW_KEY_KP_EQUAL:     return Key::NumPadEqual;

        // navigation
        case GLFW_KEY_ESCAPE:    return Key::Escape;
        case GLFW_KEY_TAB:       return Key::Tab;
        case GLFW_KEY_CAPS_LOCK: return Key::CapsLock;
        case GLFW_KEY_BACKSPACE: return Key::Backspace;
        case GLFW_KEY_ENTER:     return Key::Enter;
        case GLFW_KEY_SPACE:     return Key::Space;

        // miscellaneous
        case GLFW_KEY_INSERT:      return Key::Insert;
        case GLFW_KEY_DELETE:      return Key::Delete;
        case GLFW_KEY_HOME:        return Key::Home;
        case GLFW_KEY_END:         return Key::End;
        case GLFW_KEY_PAGE_UP:     return Key::PageUp;
        case GLFW_KEY_PAGE_DOWN:   return Key::PageDown;
        case GLFW_KEY_PRINT_SCREEN:return Key::PrintScreen;
        case GLFW_KEY_SCROLL_LOCK: return Key::ScrollLock;
        case GLFW_KEY_PAUSE:       return Key::Pause;
        case GLFW_KEY_MENU:        return Key::Menu;

        default:
            return Key::Unknown;
    }
}

KeyAction translate_glfw_action(int action) {
    switch(action) {
        case GLFW_PRESS:
            return KeyAction::Press;
        case GLFW_RELEASE:
            return KeyAction::Release;
        case GLFW_REPEAT:
            return KeyAction::Repeat;
        default:
            // GLFW should only provide the three values above.
            assert(false);
    }
}

KeyEvent KeyEvent::translate_glfw_key_event(
    int key, 
    int scancode, 
    int action, 
    int mods
) {
    return KeyEvent {
        .key = translate_glfw_key(key),
        .action = translate_glfw_action(action),
        .modifier = translate_glfw_modifiers(mods)
    };
}

// returns a char if the key event can be mapped to 
std::optional<char> KeyEvent::to_char() const {
    // only presses and repeats produce text
    if(action == KeyAction::Release) return std::nullopt;
    const bool shift = has_modifier(modifier, Modifier::Shift);
    const bool caps = has_modifier(modifier, Modifier::CapsLock);
    const bool numlock = has_modifier(modifier, Modifier::NumLock);

    // letters, caps lock and shift cancel each other out
    if(key >= Key::A && key <= Key::Z) {
        char base = static_cast<char>(static_cast<int>(key) - static_cast<int>(Key::A));
        return (shift != caps) ? char('A' + base) : char('a' + base);
    }

    // top-row digits
    if(key >= Key::Num0 && key <= Key::Num9) {
        int digit = static_cast<int>(key) - static_cast<int>(Key::Num0);
        static constexpr char SHIFTED[] = ")!@#$%^&*(";
        return shift ? SHIFTED[digit] : char('0' + digit);
    }

    // keypad digits only type when num lock is on (otherwise they navigate)
    if(key >= Key::NumPad0 && key <= Key::NumPad9) {
        if(!numlock) return std::nullopt;
        return char('0' + (static_cast<int>(key) - static_cast<int>(Key::NumPad0)));
    }

    switch(key) {
        // punctuation
        case Key::Apostrophe:   return shift ? '"' : '\'';
        case Key::Comma:        return shift ? '<' : ',';
        case Key::Minus:        return shift ? '_' : '-';
        case Key::Period:       return shift ? '>' : '.';
        case Key::Slash:        return shift ? '?' : '/';
        case Key::Semicolon:    return shift ? ':' : ';';
        case Key::Equal:        return shift ? '+' : '=';
        case Key::LeftBracket:  return shift ? '{' : '[';
        case Key::Backslash:    return shift ? '|' : '\\';
        case Key::RightBracket: return shift ? '}' : ']';
        case Key::GraveAccent:  return shift ? '~' : '`';

        // numpad operators
        case Key::NumPadDecimal:  return numlock ? std::optional<char>('.') : std::nullopt;
        case Key::NumPadDivide:   return '/';
        case Key::NumPadMultiply: return '*';
        case Key::NumPadSubtract: return '-';
        case Key::NumPadAdd:      return '+';
        case Key::NumPadEqual:    return '=';

        // spacebar
        case Key::Space: return ' ';

        default: return std::nullopt;
    }
}
    
}