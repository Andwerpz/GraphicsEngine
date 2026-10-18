#include <engine/ui/node/Viewport.h>

namespace engine::ui {

Viewport::Viewport(
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

void Viewport::update_impl(float dt) {
    // do nothing
}

void Viewport::render_impl(UIRenderer& renderer) {
    renderer.draw_textured_rect({{0, 0}, get_dim()}, *render_texture);
}

}