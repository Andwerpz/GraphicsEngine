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

#include <engine/ui/Bounds.h>

namespace engine::ui {

// TODO implement a better gaussian blurring algorithm
//   maybe something like a dual kawase / downsampling upsampling
//   need to decide on the blur sigma first though 

// responsible for efficiently rendering UI primitives to some rendering target

// UI renderer should accept draw commands like
// - draw_rect(top_left, dim)
// - draw_text(top_left, "asdf")
// the ordering of the draw commands should represent the order in which they are 'drawn' to the render target
// a primitive drawn later should go on top of any primitive drawn earlier
// when we want to actually render, we do something like
// - render()
// this should render to the underlying render target and consume all the draw commands

class UIRenderer {
    using GraphicsDevice = engine::graphics::GraphicsDevice;
    using RenderTarget = engine::graphics::RenderTarget;
    using TextureRenderTarget = engine::graphics::TextureRenderTarget;
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
    void draw_rect(Bounds rect, glm::vec4 color);
    
    // draws an axis aligned rectangle with the given texture
    void draw_textured_rect(Bounds rect, Texture& texture);

    // draws a line starting from start and ending at end
    // the end pixels are included 
    // horizontal and vertical lines of width 1 are guaranteed to be pixel perfect
    // TODO make all lines of width 1 pixel perfect
    //   look into bresenham line drawing algorithm
    void draw_line(glm::vec2 start, glm::vec2 end, float width, glm::vec4 color);

    // draws 4 axis-aligned lines outlining the given rect
    // the outline is drawn on the interior of the given rect
    // positive inset draws the outline rect smaller within the given rect
    void draw_rect_outline(Bounds rect, glm::vec4 color, float inset = 0.0f);

    // draws the given text
    // start gives the initial position of the baseline pointer 
    void draw_text(
        const std::string_view& text, 
        glm::vec2 start, 
        Font& font,
        glm::vec4 color
    );

    // blurs the given rect with a fixed kernel size gaussian blur
    void draw_blur(Bounds rect, uint32_t sigma);

    // void draw_drop_shadow();

    // push/pop origin from the origin stack
    // new origins are interpreted as relative to the current origin
    // all draw commands will be relative to the current origin
    void push_origin(glm::vec2 origin);
    void pop_origin();

    // returns the current origin in absolute space
    glm::vec2 get_origin();

    // push/pop from the clip stack 
    // new clip rectangles are interpreted as relative to the current origin
    // the new clip region is the intersection between rect and the old clip region
    void push_clip(Bounds rect);
    void pop_clip();

    // returns the current clip rect in absolute space
    Bounds get_clip();

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
    std::stack<Bounds> clip_stack;

    // origin stack
    std::stack<glm::vec2> origin_stack;

    // list of commands so far
    struct SetClip {
        Bounds rect;
    };
    struct DrawRect {
        Bounds rect;
        glm::vec4 color;
    };
    struct DrawLine {
        glm::vec2 start, end;
        float width;
        glm::vec4 color;
    };
    struct DrawTexturedRect {
        Bounds vertex_rect;
        Bounds uv_rect;
        uint64_t texture_id;
        glm::vec4 color;
    };
    struct BlurRect {
        Bounds rect;
        uint32_t sigma;
    };
    using Command = std::variant<
        SetClip,
        DrawRect,
        DrawLine,
        DrawTexturedRect,
        BlurRect
    >;
    std::vector<Command> commands;

    // mapping texture_id -> texture
    // these textures are the ones used in the current render
    std::map<uint64_t, Texture*> textures;

    // default texture/sampler to bind when nothing else is specified
    std::unique_ptr<Texture> default_texture;
    std::unique_ptr<Sampler> default_sampler;

    // render target that is used for compositing
    std::unique_ptr<Texture> compositing_texture;
    std::unique_ptr<TextureRenderTarget> compositing_target;

    // program that just renders compositing texture to the target
    std::unique_ptr<RenderingProgram> compositing_program;
    std::unique_ptr<RenderingProgramInvocation> compositing_invocation;

    // rectangle rendering pipeline
    static constexpr uint64_t RECT_INSTANCE_BUFFER_SIZE = (1 << 16);
    static constexpr uint64_t RECT_INSTANCE_SIZE = sizeof(float) * 28;
    static constexpr uint64_t RECT_INSTANCE_AMT = RECT_INSTANCE_BUFFER_SIZE / RECT_INSTANCE_SIZE;
    static_assert(RECT_INSTANCE_BUFFER_SIZE % sizeof(float) == 0);
    std::unique_ptr<Buffer> rect_instance_buffer;
    std::unique_ptr<RenderingProgram> rect_program;
    std::unique_ptr<RenderingProgramInvocation> rect_invocation;

    // mapping asset_id -> cached texture
    // these cached textures are managed by UIRenderer and are persistent across renders
    std::map<uint64_t, std::unique_ptr<Texture>> asset_textures;

    // blur pipeline
    static constexpr uint64_t BLUR_KERNEL_SIZE = 32;
    std::unique_ptr<RenderingProgram> blur_program;
    std::unique_ptr<RenderingProgramInvocation> blur_invocation_horizontal;
    std::unique_ptr<RenderingProgramInvocation> blur_invocation_vertical;
    std::unique_ptr<Texture> blur_texture;
    std::unique_ptr<TextureRenderTarget> blur_target;
};

}