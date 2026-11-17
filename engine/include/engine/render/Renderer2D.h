#pragma once
#include <stack>

#include <engine/util/macro.h>
#include <engine/math/Rect.h>

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

namespace engine::render {

// Renderer2D is meant to render stuff to a given RenderTarget
// it is assumed that the provided RenderTarget only has a color buffer in attachment 0

// for basic rendering, Renderer2D provides some basic primitives such as axis aligned quads

// to allow the user to perform arbitrary shading, we let the user pass in custom fragment shaders
// that are compatible with the vertex shaders provided by Renderer2D and the bound RenderTarget
// essentially, Renderer2D decides what pixels get rendered to, the user decides how to color the pixels

// a Material encapsulates a fragment shader + what is supposed to be bound to that fragment shader
// two rendering calls can be batched if they use the same primitive and their materials are the same

// for even more arbitrary rendering, like if you want to do:
// - rendering to intermediate buffers
// - taking in the current color buffer so far as a texture
// you will need to define a custom rendering pass. 
// it should take in a Texture representing the current output buffer and a RenderTarget to render to.
// since custom rendering passes can be doing anything, they will not be batched.


// TODO actually implement arbitrary shading on top of primitives

// TODO currently, we pass in the entire background texture and render target to a custom pass
//   consider making a version of the custom pass where we only pass in a view of the texture and render target
//   this is so that custom pass implementations do not need to worry about offsetting themselves

class Renderer2D {
    using Rect2 = engine::math::Rect2;
    using GraphicsDevice = engine::graphics::GraphicsDevice;
    using RenderTarget = engine::graphics::RenderTarget;
    using TextureRenderTarget = engine::graphics::TextureRenderTarget;
    using RenderingProgram = engine::graphics::RenderingProgram;
    using RenderingProgramInvocation = engine::graphics::RenderingProgramInvocation;
    using Buffer = engine::graphics::Buffer;
    using Texture = engine::graphics::Texture;
    using Sampler = engine::graphics::Sampler;

public:
    // represents an arbitrary rendering pass done by the user
    // we pass in a Texture representing the current color buffer, and a RenderTarget to render to
    using CustomPass = std::function<void(Texture&, RenderTarget&)>;

    Renderer2D(
        GraphicsDevice& device,
        RenderTarget& target
    );
    NON_COPYABLE_NON_MOVABLE(Renderer2D);

    // two quad draw commands can be batched together if they share the same material
    // for now, just holds the texture that is supposed to be bound
    // a Material is meant to be lightweight and copied around
    // TODO fragment shader + full list of stuff bound to the fragment shader
    struct Material {
        Texture* texture = nullptr;   // nullptr indicates binding to the default texture
    };

    // quad instances are uploaded to the GPU as
    //   mat4 : model matrix transforming [0, 1] x [0, 1]
    //   vec4 : {uv_pos.x, uv_pos.y, uv_dim.x, uv_dim.y}
    //   vec4 : premultiplied color
    struct QuadInstance {
        glm::mat4 model;
        Rect2 uv = Rect2 {{0, 0}, {1, 1}};
        glm::vec4 color = glm::vec4 {1, 1, 1, 1};
    };

    // draw quad helpers
    void draw_quad(glm::mat4 model, glm::vec4 color);
    void draw_quad(
        glm::mat4 model,
        Texture& texture,
        glm::vec4 color = {1, 1, 1, 1},
        Rect2 uv = {{0, 0}, {1, 1}}
    );

    // draw axis + pixel aligned quad helpers
    // a pixel is covered by a rect if its center lies inside the rect
    void draw_rect(Rect2 rect, glm::vec4 color);
    void draw_rect(
        Rect2 rect,
        Texture& texture,
        glm::vec4 color = {1, 1, 1, 1},
        Rect2 uv = {{0, 0}, {1, 1}}
    );

    // custom rendering pass
    void draw_custom_pass(CustomPass pass);

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
    void push_clip(Rect2 rect);
    void pop_clip();

    // render to the bound target using the queued commands and resets the renderer
    void render();

    // resets the renderer
    void reset();

protected:
    // pushes a quad to the quad rendering pipeline
    void emit_quad(const QuadInstance& quad, const Material& material);

    // push arbitrary quad to rendering pipeline
    void push_quad(glm::mat4 model, Rect2 uv, glm::vec4 color, Material material);
    
    // push axis + pixel aligned quad to rendering pipeline
    void push_rect(Rect2 rect, Rect2 uv, glm::vec4 color, Material material);

    // underlying graphics device used for rendering
    GraphicsDevice& device;

    // target to render to
    RenderTarget& target;

    // clip rectangle stack
    std::stack<Rect2> clip_stack;

    // origin stack
    std::stack<glm::vec2> origin_stack;

    // quad rendering pipeline
    std::vector<QuadInstance> quad_instances;
    std::unique_ptr<Buffer> quad_instance_buffer;
    std::unique_ptr<RenderingProgram> quad_program;
    std::unique_ptr<RenderingProgramInvocation> quad_drawer;
    static constexpr uint64_t QUAD_INSTANCE_SIZE = sizeof(float) * (16 + 4 + 4);
    static constexpr uint64_t QUAD_INSTANCE_CAPACITY = 1 << 14;
    static constexpr uint64_t QUAD_BUFFER_SIZE = QUAD_INSTANCE_SIZE * QUAD_INSTANCE_CAPACITY;
    static_assert(sizeof(Rect2) == sizeof(float) * 4);
    static_assert(sizeof(QuadInstance) == QUAD_INSTANCE_SIZE);

    // should set the clip rect to whatever clip_rect is
    struct SetClip {
        Rect2 clip_rect;
    };
    
    // draw a batch of instanced quads
    // refers to a subrange within the quad_instances vector
    struct DrawQuadBatch {
        size_t start;
        size_t amt;
        Material material;
    };

    // all commands queued so far
    // each command will get executed independently
    using Command = std::variant<
        SetClip,
        DrawQuadBatch,
        CustomPass
    >;
    std::vector<Command> commands;

    // default texture/sampler to bind when nothing else is specified
    std::unique_ptr<Texture> default_texture;   // 1x1 white texture
    std::unique_ptr<Sampler> default_sampler;   // linear sampler, clamp to edge

    // render target that is used for compositing
    std::unique_ptr<Texture> compositing_texture;
    std::unique_ptr<TextureRenderTarget> compositing_target;

    // program that just renders compositing texture to the target
    std::unique_ptr<RenderingProgram> compositing_program;
    std::unique_ptr<RenderingProgramInvocation> compositing_to_target;

    // custom passes are given scratch_target to render to
    // scratch_texture rendered back to the compositing target so that we can enforce clipping
    std::unique_ptr<Texture> scratch_texture;
    std::unique_ptr<TextureRenderTarget> scratch_target;

    // program to copy from scratch to compositing
    std::unique_ptr<RenderingProgram> scratch_program;
    std::unique_ptr<RenderingProgramInvocation> scratch_to_compositing;
};

}