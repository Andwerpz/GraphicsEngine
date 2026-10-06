#pragma once

#include <engine/graphics/GraphicsDevice.h>
#include <engine/graphics/RenderTarget.h>

namespace engine::ui {

// responsible for efficiently rendering UI primitives to some rendering target

// UI renderer should accept draw commands like
// - draw_rect(top_left, dim)
// - draw_text(top_left, "asdf")
// the ordering of the draw commands should represent the order in which they are 'drawn' to the render target
// a primitive drawn later should go on top of any primitive drawn earlier
// when we want to actually render, we do something like
// - render(render_target)
// this should render to the given target and consume all the draw commands

// what primitives should the UI renderer be able to render?
// - rectangle
// - text
// - line
// - textured rectangle

// it should also have a clip rectangle stack
// - push_clip(top_left, dim)
// - pop_clip()

class UIRenderer {
    using GraphicsDevice = engine::graphics::GraphicsDevice;
    using RenderTarget = engine::graphics::RenderTarget;

public:
    UIRenderer(GraphicsDevice& device) : device(device) {}

    void draw_rect();
    void draw_text();

    void push_clip();
    void pop_clip();

    void render(RenderTarget& target);

private:
    // underlying graphics device used for rendering
    GraphicsDevice& device;

    // list of commands so far

    // clip rectangle stack
};

}