#pragma once
#include <engine/graphics/GraphicsDevice.h>
#include <engine/graphics/RenderTarget.h>
#include <engine/graphics/Program.h>
#include <engine/graphics/ProgramInvocation.h>
#include <stack>

namespace engine::ui {

// responsible for efficiently rendering UI primitives to some rendering target

// UI renderer should accept draw commands like
// - draw_rect(top_left, dim)
// - draw_text(top_left, "asdf")
// the ordering of the draw commands should represent the order in which they are 'drawn' to the render target
// a primitive drawn later should go on top of any primitive drawn earlier
// when we want to actually render, we do something like
// - render()
// this should render to the underlying render target and consume all the draw commands

// what primitives should the UI renderer be able to render?
// - rectangle
// - line
// - text
// - textured rectangle

// shader effects?
// - blur a rectangle
// - rectangular drop shadow

// it should also have a clip rectangle stack
// - push_clip(top_left, dim)
// - pop_clip()

class UIRenderer {
    using GraphicsDevice = engine::graphics::GraphicsDevice;
    using RenderTarget = engine::graphics::RenderTarget;
    using RenderingProgram = engine::graphics::RenderingProgram;
    using RenderingProgramInvocation = engine::graphics::RenderingProgramInvocation;
    using Buffer = engine::graphics::Buffer;

public:
    UIRenderer(
        GraphicsDevice& device,
        RenderTarget& target
    );

    struct Color {
        float r, g, b, a;
    };
    struct Rect {
        float x, y;
        float width, height;
    };
    struct Vec2 {
        float x, y;
    };

    // draw commands
    void draw_rect(Rect rect, Color color);
    void draw_line(Vec2 start, Vec2 end, float width, Color color);

    // void draw_text();
    // void draw_blur();
    // void draw_drop_shadow();

    // push/pop from the clip stack 
    // clip rectangles are always absolute
    // the new clip region is the intersection between rect and the old clip region
    // these should also turn into 'commands'
    void push_clip(Rect rect);
    void pop_clip();

    // render to target using commands
    void render();

    // resets the renderer to before any commands were given
    void reset();

private:
    // underlying graphics device used for rendering
    GraphicsDevice& device;

    // target to render to
    RenderTarget& target;

    // clip rectangle stack
    std::stack<Rect> clip_stack;

    // list of commands so far
    struct SetClip {
        Rect rect;
    };
    struct DrawRect {
        Rect rect;
        Color color;
    };
    struct DrawLine {
        Vec2 start, end;
        float width;
        Color color;
    };
    using Command = std::variant<
        SetClip,
        DrawRect,
        DrawLine
    >;
    std::vector<Command> commands;

    // resources to render primitives
    const uint64_t INSTANCE_BUFFER_SIZE = (1 << 16);
    std::unique_ptr<Buffer> instance_buffer;
    std::unique_ptr<RenderingProgram> rect_program;
    std::unique_ptr<RenderingProgramInvocation> rect_invocation;
};

}