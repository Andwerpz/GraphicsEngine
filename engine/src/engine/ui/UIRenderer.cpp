#include <engine/ui/UIRenderer.h>
#include <engine/util/error.h>
#include <engine/asset/EmbeddedResource.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
namespace fs = std::filesystem;

#include <iostream>

namespace engine::ui {

UIRenderer::UIRenderer(
    GraphicsDevice& device,
    RenderTarget& target
) : engine::render::Renderer2D(device, target) {
    // reset();

    // create stuff needed to render
    using namespace engine::graphics;
    uint32_t width = target.get_width();
    uint32_t height = target.get_height();

    auto create_program = [&](const std::string& name) -> std::unique_ptr<RenderingProgram> {
        auto vertex_source = engine::asset::load_asset("engine://res/shaders/ui/" + name + ".vert");
        auto fragment_source = engine::asset::load_asset("engine://res/shaders/ui/" + name + ".frag");
        auto vertex_shader = device.create_shader(
            std::string_view((char*) vertex_source.data(), vertex_source.size()),
            ShaderStage::Vertex
        );
        auto fragment_shader = device.create_shader(
            std::string_view((char*) fragment_source.data(), fragment_source.size()),
            ShaderStage::Fragment
        );
        return device.create_rendering_program(
            *vertex_shader, *fragment_shader,
            PrimitiveTopology::TriangleList
        );
    };

    // blur
    {
        blur_texture = device.create_texture(
            TextureDimension::D2,
            TextureFormat::RGBA32F,
            width, height, 1, 1
        );
        blur_target = device.create_render_target(width, height);
        blur_target->bind_texture(0, *blur_texture);
        blur_target->validate();

        blur_program = create_program("blur");
        
        // compositing texture -> blur texture
        blur_invocation_horizontal = blur_program->create_invocation();
        blur_invocation_horizontal->bind_render_target(*blur_target);
        blur_invocation_horizontal->bind_sampled_texture({0, 1}, *compositing_texture, *default_sampler);
        blur_invocation_horizontal->set_uniform("settings.is_vertical", 0);
        blur_invocation_horizontal->do_clear_color(true);

        // blur texture -> compositing texture
        blur_invocation_vertical = blur_program->create_invocation();
        blur_invocation_vertical->bind_render_target(*compositing_target);
        blur_invocation_vertical->bind_sampled_texture({0, 1}, *blur_texture, *default_sampler);
        blur_invocation_vertical->set_uniform("settings.is_vertical", 1);
        blur_invocation_vertical->do_clear_color(false);
        blur_invocation_vertical->do_clipping(true);
    }
}

void UIRenderer::draw_line(
    glm::vec2 start, 
    glm::vec2 end, 
    float width, 
    glm::vec4 color
) {
    auto snap_center = [](float v, float w) -> float {
        return (static_cast<int>(w) % 2 == 1) ? std::floor(v) + 0.5f : std::round(v);
    };
    float dx = end.x - start.x;
    float dy = end.y - start.y;
    const float eps = 1e-4f;

    glm::mat4 transform(1.0f);
    if(std::abs(dy) < eps) {                            // horizontal
        float y = snap_center(start.y, width);
        float x0 = std::round(std::min(start.x, end.x));
        float x1 = std::round(std::max(start.x, end.x)) + 1.0;  // include last pixel
        transform = glm::translate(glm::mat4(1.0f), glm::vec3(x0, y - width / 2.0f, 0.0f));
        transform = glm::scale(transform, glm::vec3(x1 - x0, width, 1.0f));
    }
    else if(std::abs(dx) < eps) {                       // vertical
        float x = snap_center(start.x, width);
        float y0 = std::round(std::min(start.y, end.y));
        float y1 = std::round(std::max(start.y, end.y)) + 1.0;  // include last pixel
        transform = glm::translate(glm::mat4(1.0f), glm::vec3(x - width / 2.0f, y0, 0.0f));
        transform = glm::scale(transform, glm::vec3(width, y1 - y0, 1.0f));
    }
    else {                                              // diagonal
        glm::vec2 a{start.x + 0.5f, start.y + 0.5f};
        glm::vec2 b{end.x + 0.5f,   end.y + 0.5f};

        glm::vec2 d = b - a;
        float len = glm::length(d);
        if(len > 1e-6f) {
            // extend half a pixel past each end
            glm::vec2 dir = d / len;
            a -= dir * 0.5f;            
            b += dir * 0.5f;
        }

        float length = glm::length(b - a);
        float angle  = std::atan2(b.y - a.y, b.x - a.x);
        transform = glm::translate(glm::mat4(1.0f), glm::vec3(a.x, a.y, 0.0f));
        transform = glm::rotate(transform, angle, glm::vec3(0, 0, 1));
        transform = glm::translate(transform, glm::vec3(0.0f, -width / 2.0f, 0.0f));
        transform = glm::scale(transform, glm::vec3(length, width, 1.0f));
    }

    draw_quad(transform, color);
}

void UIRenderer::draw_rect_outline(Rect2 rect, glm::vec4 color, float inset) {
    rect.dim = rect.dim - glm::vec2(inset * 2.0f);
    glm::vec2 c0 = rect.pos + glm::vec2(inset);
    glm::vec2 c1 = c0;  c1.x += rect.dim.x - 1;
    glm::vec2 c2 = c1;  c2.y += rect.dim.y - 1;
    glm::vec2 c3 = c2;  c3.x -= rect.dim.x - 1;
    draw_line(c0, c1, 1, color);
    draw_line(c1, c2, 1, color);
    draw_line(c2, c3, 1, color);
    draw_line(c3, c0, 1, color);
}

void UIRenderer::draw_text(
    const std::string_view& text, 
    glm::vec2 start, 
    Font& font,
    glm::vec4 color
) {
    // get font atlas texture
    Image& atlas = font.get_atlas();
    uint64_t asset_id = atlas.get_asset_id();
    if(!asset_textures.contains(asset_id)) {
        auto texture = device.create_texture(
            engine::graphics::TextureDimension::D2,
            engine::graphics::TextureFormat::RGBA32F,
            atlas.get_width(),
            atlas.get_height(),
            1,
            1
        );
        texture->upload_image(atlas);
        asset_textures.emplace(asset_id, std::move(texture));
    }
    Texture& atlas_texture = *asset_textures.at(asset_id);
    uint64_t texture_id = atlas_texture.get_texture_id();

    // rect commands
    glm::vec2 pen = start;
    glm::vec2 atlas_size = {atlas.get_width(), atlas.get_height()};
    for(int i = 0; i < text.size(); i++) {
        char c = text[i];
        Font::Glyph& glyph = font.get_glyph(c);

        glm::vec2 pos = pen;
        pos.x += glyph.bearing_x;
        pos.y += glyph.bearing_y;

        // need to round to nearest pixel to not let sampling mess with text sharpness
        pos = glm::round(pos);

        glm::vec2 dim = {glyph.width, glyph.height};
        dim += glm::vec2 {0.5, 0.5};

        glm::vec2 uv_pos = {glyph.atlas_x, glyph.atlas_y};
        uv_pos.x /= atlas.get_width();
        uv_pos.y /= atlas.get_height();
        glm::vec2 uv_dim = dim;
        uv_dim.x /= atlas.get_width();
        uv_dim.y /= atlas.get_height();

        draw_rect({pos, dim}, atlas_texture, color, {uv_pos, uv_dim});        
        pen.x += glyph.advance_x;
    }
}

void UIRenderer::draw_blur(Rect2 rect, uint32_t sigma) {
    rect.pos += get_origin();

    auto blur_pass = [this, rect, sigma](Texture& background, RenderTarget& pass_target) -> void {
        glm::vec2 target_size = {target.get_width(), target.get_height()};
        glm::vec2 uv_pos = rect.pos / target_size;
        glm::vec2 uv_dim = rect.dim / target_size;

        // background -> blur texture
        blur_invocation_horizontal->bind_sampled_texture({0, 1}, background, *default_sampler);
        blur_invocation_horizontal->set_uniform<glm::vec2>("settings.pos", uv_pos);
        blur_invocation_horizontal->set_uniform<glm::vec2>("settings.dim", uv_dim);
        blur_invocation_horizontal->set_uniform<int>("settings.sigma", sigma);
        blur_invocation_horizontal->draw(1);

        // blur texture -> pass_target
        blur_invocation_vertical->bind_render_target(pass_target);
        blur_invocation_vertical->set_uniform<glm::vec2>("settings.pos", uv_pos);
        blur_invocation_vertical->set_uniform<glm::vec2>("settings.dim", uv_dim);
        blur_invocation_vertical->set_uniform<int>("settings.sigma", sigma);
        blur_invocation_vertical->draw(1);
    };
    draw_custom_pass(blur_pass);
}
    
}