#pragma once
#include <cassert>
#include <cstdint>
#include <utility>

#include "../Window.h"
#include "GLContext.h"

namespace engine::graphics::opengl {

// represents an GLFW OS window 

class GLGraphicsDevice;

class GLWindow : public engine::graphics::Window {
    friend class GLGraphicsDevice;

public:
    GLWindow(GLWindow&& other);
    ~GLWindow() = default;

    GLWindow() = delete;

    uint32_t get_width() override;
    uint32_t get_height() override;

    GLContext& get_context() {
        return context;
    }

    void swap_buffers();

private:
    GLWindow(
        GLGraphicsDevice& device,
        GLContext&& context
    ) : device(device), context(std::move(context)) {}

    // the graphics device that created this window
    GLGraphicsDevice& device;

    // this window owns the OpenGL context 
    GLContext context;
};

}
