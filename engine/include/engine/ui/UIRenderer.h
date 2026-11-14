#pragma once
#include <stack>
#include <map>
#include <memory>

#include <glm/glm.hpp>

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

#include <engine/render/Renderer2D.h>

namespace engine::ui {

// TODO implement a better gaussian blurring algorithm
//   maybe something like a dual kawase / downsampling upsampling
//   need to decide on the blur sigma first though 

// specialized 2D renderer meant for drawing UI primitives
class UIRenderer : public engine::render::Renderer2D {
    using Rect2 = engine::math::Rect2;
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
    NON_COPYABLE_NON_MOVABLE(UIRenderer);

    // draws an axis aligned rectangle with the given image as the texture
    void draw_image_rect(
        Rect2 rect, 
        Image& image, 
        Rect2 uv_rect = {{0, 0}, {1, 1}}
    );

    // draws a line starting from start and ending at end
    // the end pixels are included 
    // horizontal and vertical lines of width 1 are guaranteed to be pixel perfect
    // TODO make all lines of width 1 pixel perfect
    //   look into bresenham line drawing algorithm
    void draw_line(glm::vec2 start, glm::vec2 end, float width, glm::vec4 color);

    // draws 4 axis-aligned lines outlining the given rect
    // the outline is drawn on the interior of the given rect
    // positive inset draws the outline rect smaller within the given rect
    void draw_rect_outline(Rect2 rect, glm::vec4 color, float inset = 0.0f);

    // draws the given text
    // start gives the initial position of the baseline pointer 
    void draw_text(
        const std::string_view& text, 
        glm::vec2 start, 
        Font& font,
        glm::vec4 color
    );

    // blurs the given rect with a fixed kernel size gaussian blur
    void draw_blur(Rect2 rect, uint32_t sigma);

    // TODO
    // void draw_drop_shadow();

private:
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