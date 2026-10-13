#pragma once
#include <stack>
#include <map>
#include <memory>

#include <glm/glm.hpp>

#include <engine/graphics/GraphicsDevice.h>
#include <engine/graphics/RenderTarget.h>
#include <engine/graphics/Program.h>
#include <engine/graphics/ProgramInvocation.h>
#include <engine/graphics/Shader.h>
#include <engine/graphics/Texture.h>
#include <engine/graphics/Sampler.h>
#include <engine/graphics/Buffer.h>

#include <engine/asset/Font.h>
#include <engine/asset/Image.h>

#include <engine/ui/UIRect.h>

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
    using Texture = engine::graphics::Texture;
    using Sampler = engine::graphics::Sampler;
    using Font = engine::asset::Font;
    using Image = engine::asset::Image;

public:
    UIRenderer(
        GraphicsDevice& device,
        RenderTarget& target
    );

    // draws an axis aligned rectangle with solid color
    void draw_rect(UIRect rect, glm::vec4 color);

    // draws a line starting from start and ending at end
    // the end pixels are included 
    // horizontal and vertical lines of width 1 are guaranteed to be pixel perfect
    // TODO make all lines of width 1 pixel perfect
    //   look into bresenham line drawing algorithm
    void draw_line(glm::vec2 start, glm::vec2 end, float width, glm::vec4 color);

    // draws the given text
    // start gives the initial position of the baseline pointer 
    void draw_text(
        const std::string_view& text, 
        glm::vec2 start, 
        Font& font
    );

    // void draw_blur();
    // void draw_drop_shadow();

    // push/pop from the clip stack 
    // clip rectangles are always absolute
    // the new clip region is the intersection between rect and the old clip region
    // these should also turn into 'commands'
    void push_clip(UIRect rect);
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
    std::stack<UIRect> clip_stack;

    // list of commands so far
    struct SetClip {
        UIRect rect;
    };
    struct DrawRect {
        UIRect rect;
        glm::vec4 color;
    };
    struct DrawLine {
        glm::vec2 start, end;
        float width;
        glm::vec4 color;
    };
    struct DrawTexturedRect {
        UIRect vertex_rect;
        UIRect uv_rect;
        uint64_t asset_id;
    };
    using Command = std::variant<
        SetClip,
        DrawRect,
        DrawLine,
        DrawTexturedRect
    >;
    std::vector<Command> commands;

    // rectangle rendering pipeline
    const uint64_t INSTANCE_BUFFER_SIZE = (1 << 16);
    const uint64_t INSTANCE_SIZE = sizeof(float) * 28;
    std::unique_ptr<Buffer> instance_buffer;
    std::unique_ptr<RenderingProgram> rect_program;
    std::unique_ptr<RenderingProgramInvocation> rect_invocation;

    // default texture stuff to bind when nothing else is specified
    std::unique_ptr<Texture> default_texture;
    std::unique_ptr<Sampler> default_sampler;

    // mapping asset_id -> cached texture
    std::map<uint64_t, std::unique_ptr<Texture>> asset_textures;
};

}