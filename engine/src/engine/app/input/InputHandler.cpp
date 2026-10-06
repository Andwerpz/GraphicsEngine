#include <engine/app/input/InputHandler.h>
#include <engine/app/TaskLoop.h>
#include <engine/graphics/opengl/GLWindow.h>
#include <engine/graphics/opengl/GLContext.h>
#include <engine/utils/error.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace engine::app {

InputHandler::~InputHandler() {
    // remove GLFW input polling task
    // TODO
}

void InputHandler::add_window(Window& window) {
    if(contains(window)) return;
    windows.insert(&window);

    if(dynamic_cast<engine::graphics::opengl::GLWindow*>(&window) != nullptr) {
        // GLFW window
        auto& gl_window = dynamic_cast<engine::graphics::opengl::GLWindow&>(window);

        // create callback context struct
        GLFWCallbackContext* glfw_context = new GLFWCallbackContext();
        glfw_context->handler = this;
        glfw_context->window = &window;
        glfw_contexts.insert({&window, glfw_context});

        // install GLFW callback handlers
        GLFWwindow* glfw_window = gl_window.get_context().get_glfw_window();
        glfwSetWindowUserPointer(glfw_window, glfw_context);

        glfwSetCursorPosCallback(
            glfw_window,
            [](GLFWwindow* glfw_window, double xpos, double ypos) -> void {
                GLFWCallbackContext* context = (GLFWCallbackContext*) glfwGetWindowUserPointer(glfw_window);
                assert(context != nullptr);
                InputHandler* handler = context->handler;
                Window* window = context->window;
                if(handler->cursor_position_callbacks.contains(window)) {
                    auto func = handler->cursor_position_callbacks.at(window);
                    CursorPositionEvent ev;
                    ev.xpos = xpos;
                    ev.ypos = ypos;
                    func(ev);
                }
            }
        );
        glfwSetMouseButtonCallback(
            glfw_window,
            [](GLFWwindow* glfw_window, int button, int action, int mods) -> void {
                GLFWCallbackContext* context = (GLFWCallbackContext*) glfwGetWindowUserPointer(glfw_window);
                assert(context != nullptr);
                InputHandler* handler = context->handler;
                Window* window = context->window;
                if(handler->mouse_button_callbacks.contains(window)) {
                    auto func = handler->mouse_button_callbacks.at(window);
                    MouseButtonEvent ev = MouseButtonEvent::translate_glfw_mouse_button_event(button, action, mods);
                    func(ev);
                }
            }
        );
        glfwSetScrollCallback(
            glfw_window,
            [](GLFWwindow* glfw_window, double xoffset, double yoffset) -> void {
                GLFWCallbackContext* context = (GLFWCallbackContext*) glfwGetWindowUserPointer(glfw_window);
                assert(context != nullptr);
                InputHandler* handler = context->handler;
                Window* window = context->window;
                if(handler->scroll_callbacks.contains(window)) {
                    auto func = handler->scroll_callbacks.at(window);
                    ScrollEvent ev;
                    ev.xoff = xoffset;
                    ev.yoff = yoffset;
                    func(ev);
                }
            }
        );
        glfwSetKeyCallback(
            glfw_window,
            [](GLFWwindow* glfw_window, int key, int scancode, int action, int mods) -> void {
                GLFWCallbackContext* context = (GLFWCallbackContext*) glfwGetWindowUserPointer(glfw_window);
                assert(context != nullptr);
                InputHandler* handler = context->handler;
                Window* window = context->window;
                if(handler->key_callbacks.contains(window)) {
                    auto func = handler->key_callbacks.at(window);
                    KeyEvent ev = KeyEvent::translate_glfw_key_event(key, scancode, action, mods);
                    func(ev);
                }
            }
        );

        // install glfw poller
        install_glfw_polling_task();
    }
    else assert(false);
}

bool InputHandler::contains(Window& window) {
    for(Window* _window : windows) {
        if(_window == &window) {
            return true;
        }
    }
    return false;
}

void InputHandler::set_cursor_position_callback(
    Window& window, 
    CursorPositionCallback_t func
) {
    // make sure we're managing this window
    if(!contains(window)) {
        engine::utils::throw_error("input handler needs to manage window before adding callback");
    }
    
    // add callback
    cursor_position_callbacks.insert({&window, func});
}

void InputHandler::set_mouse_button_callback(
    Window& window, 
    MouseButtonCallback_t func
) {
    // make sure we're managing this window
    if(!contains(window)) {
        engine::utils::throw_error("input handler needs to manage window before adding callback");
    }

    // add callback
    mouse_button_callbacks.insert({&window, func});
}

void InputHandler::set_scroll_callback(
    Window& window,
    ScrollCallback_t func
) {
    // make sure we're managing this window
    if(!contains(window)) {
        engine::utils::throw_error("input handler needs to manage window before adding callback");
    }

    // add callback
    scroll_callbacks.insert({&window, func});
}

void InputHandler::set_key_callback(
    Window& window,
    KeyCallback_t func
) {
    // make sure we're managing this window
    if(!contains(window)) {
        engine::utils::throw_error("input handler needs to manage window before adding callback");
    }

    // add callback
    key_callbacks.insert({&window, func});
}

void InputHandler::install_glfw_polling_task() {
    if(installed_glfw_polling_task) return;
    task_loop.add_periodic_task(
        "InputHandler GLFW Event Poller",
        []() -> void {
            glfwPollEvents();
        },
        std::chrono::milliseconds(1)
    );
    installed_glfw_polling_task = true;
}

}