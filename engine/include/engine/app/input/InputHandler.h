#pragma once
#include <engine/graphics/Window.h>
#include <functional>
#include <set>
#include <map>

#include "KeyEvent.h"
#include "MouseButtonEvent.h"

namespace engine::app {

// an InputHandler manages user input callbacks for graphics windows 
// it owns the callbacks and internally queues up the user input events
// it attaches to a task loop and creates reactive tasks that run to drain the user input events

class TaskLoop;

struct CursorPositionEvent {
    // position is given relative to the top left corner of the window
    double xpos;
    double ypos;
};

struct ScrollEvent {
    double xoff;    // positive indicates scrolling right
    double yoff;    // positive indicates scrolling up
};

class InputHandler {
    using Window = engine::graphics::Window;
    using CursorPositionCallback_t = std::function<void(const CursorPositionEvent&)>;
    using MouseButtonCallback_t = std::function<void(const MouseButtonEvent&)>;
    using ScrollCallback_t = std::function<void(const ScrollEvent&)>;
    using KeyCallback_t = std::function<void(const KeyEvent&)>;

public:
    InputHandler(TaskLoop& task_loop) : task_loop(task_loop) {}
    ~InputHandler();

    // register a window to be managed by this handler
    void add_window(Window& window);

    // returns true if this handler is handling the given window
    bool contains(Window& window);

    // register callbacks for input events received by this handler
    void set_cursor_position_callback(
        Window& window, 
        CursorPositionCallback_t func
    );
    void set_mouse_button_callback(
        Window& window, 
        MouseButtonCallback_t func
    );
    void set_scroll_callback(
        Window& window,
        ScrollCallback_t func
    );
    void set_key_callback(
        Window& window,
        KeyCallback_t func
    );

private:
    bool installed_glfw_polling_task = false;
    void install_glfw_polling_task();

    // task loop this handler is attached to
    TaskLoop& task_loop;

    // list of windows being handled
    std::set<Window*> windows;

    // mapping windows to callbacks
    std::map<Window*, CursorPositionCallback_t> cursor_position_callbacks;
    std::map<Window*, MouseButtonCallback_t> mouse_button_callbacks;
    std::map<Window*, ScrollCallback_t> scroll_callbacks;
    std::map<Window*, KeyCallback_t> key_callbacks;

    // allocated when installing GLFW callbacks 
    // GLFW allows you to specify one void* that you can access in the callback
    // just give it a pointer to one of these structs so we can look up the user callbacks
    struct GLFWCallbackContext {
        InputHandler* handler;
        Window* window;
    };
    std::map<Window*, GLFWCallbackContext*> glfw_contexts;
};

}