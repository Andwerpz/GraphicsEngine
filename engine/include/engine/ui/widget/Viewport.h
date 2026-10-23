#pragma once
#include <memory>
#include <engine/graphics/GraphicsDevice.h>
#include <engine/graphics/Texture.h>
#include <engine/graphics/RenderTarget.h>
#include <engine/ui/UINode.h>

namespace engine::ui {

// a Viewport should provide a surface to do arbitrary rendering on
// this surface should be then rendered as part of the UI

class Viewport : public UINode {
    using GraphicsDevice = engine::graphics::GraphicsDevice;
    using Texture = engine::graphics::Texture;
    using RenderTarget = engine::graphics::RenderTarget;
    using TextureRenderTarget = engine::graphics::TextureRenderTarget;

public:
    Viewport(
        UIContext& context,
        GraphicsDevice& device,
        glm::vec2 dim
    ) : UINode(context),
        device(device)
    {
        set_fixed_dim(dim);

        // initialize render target
        using TextureDimension = engine::graphics::TextureDimension;
        using TextureFormat = engine::graphics::TextureFormat;
        uint32_t width = std::max((uint32_t) 1, (uint32_t) dim.x);
        uint32_t height = std::max((uint32_t) 1, (uint32_t) dim.y);
        render_texture = device.create_texture(
            TextureDimension::D2,
            TextureFormat::RGBA32F,
            width, height,
            1, 1
        );
        render_target = device.create_render_target(width, height);
        render_target->bind_texture(0, *render_texture);
        render_target->bind_depth_buffer();
        render_target->validate();
    }

    Texture& get_render_texture() {
        return *render_texture;
    }
    
    RenderTarget& get_render_target() {
        return *render_target;
    }

private:
    void render_content(UIRenderer& renderer) override {
        renderer.draw_textured_rect({{0, 0}, get_dim()}, *render_texture);
    }

    GraphicsDevice& device;
    std::unique_ptr<Texture> render_texture;
    std::unique_ptr<TextureRenderTarget> render_target;
};

}