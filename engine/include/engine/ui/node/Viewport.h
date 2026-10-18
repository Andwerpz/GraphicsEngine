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
    );

    Texture& get_render_texture() {
        return *render_texture;
    }
    
    RenderTarget& get_render_target() {
        return *render_target;
    }

private:
    void update_impl(float dt) override;
    void render_impl(UIRenderer& renderer) override;

    GraphicsDevice& device;
    std::unique_ptr<Texture> render_texture;
    std::unique_ptr<TextureRenderTarget> render_target;
};

}