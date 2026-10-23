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
) : device(device), 
    target(target) 
{
    reset();

    // create stuff needed to render
    using Program = engine::graphics::Program;
    using ProgramInvocation = engine::graphics::ProgramInvocation;
    using Shader = engine::graphics::Shader;
    using ShaderStage = engine::graphics::ShaderStage;
    using PrimitiveTopology = engine::graphics::PrimitiveTopology;
    using BufferUsage = engine::graphics::BufferUsage;
    using BufferFrequency = engine::graphics::BufferFrequency;
    using VertexInputRate = engine::graphics::VertexInputRate;
    using VertexFormat = engine::graphics::VertexFormat;
    using TextureDimension = engine::graphics::TextureDimension;
    using TextureFormat = engine::graphics::TextureFormat;
    using TextureRenderTarget = engine::graphics::TextureRenderTarget;

    uint32_t width = target.get_width();
    uint32_t height = target.get_height();

    default_texture = device.create_texture(
        engine::graphics::TextureDimension::D2,
        engine::graphics::TextureFormat::RGBA32F,
        1, 1, 1, 1
    );
    default_sampler = device.create_sampler();

    auto create_program = [&](const std::string& name) -> std::unique_ptr<RenderingProgram> {
        auto vertex_source = asset::EmbeddedResource::load("res/shaders/ui/" + name + ".vert");
        auto fragment_source = asset::EmbeddedResource::load("res/shaders/ui/" + name + ".frag");
        auto vertex_shader = device.create_shader(
            std::string_view((char*) vertex_source.data, vertex_source.size),
            ShaderStage::Vertex
        );
        auto fragment_shader = device.create_shader(
            std::string_view((char*) fragment_source.data, fragment_source.size),
            ShaderStage::Fragment
        );
        return device.create_rendering_program(
            *vertex_shader, *fragment_shader,
            PrimitiveTopology::TriangleList
        );
    };

    // compositing
    {
        compositing_texture = device.create_texture(
            TextureDimension::D2,
            TextureFormat::RGBA32F,
            width, height, 1, 1
        );
        compositing_target = device.create_render_target(width, height);
        compositing_target->bind_texture(0, *compositing_texture);
        compositing_target->bind_depth_buffer();
        compositing_target->validate();

        compositing_program = create_program("compositing");
        compositing_invocation = compositing_program->create_invocation();
        compositing_invocation->bind_render_target(target);
        compositing_invocation->validate();

        compositing_invocation->bind_sampled_texture(
            {0, 0}, *compositing_texture, *default_sampler
        );
    }

    // rect
    {
        rect_instance_buffer = device.create_buffer(
            RECT_INSTANCE_BUFFER_SIZE, 
            BufferUsage::Vertex, 
            BufferFrequency::Dynamic
        );

        rect_program = create_program("rect");
        rect_invocation = rect_program->create_invocation();
        rect_invocation->bind_render_target(*compositing_target);
        rect_invocation->bind_vertex_attribute(     // instance transform
            0,
            *rect_instance_buffer,
            0,
            RECT_INSTANCE_SIZE,
            VertexInputRate::Instance,
            VertexFormat::Float32x4x4
        );
        rect_invocation->bind_vertex_attribute(     // instance color
            4,
            *rect_instance_buffer,
            4 * 16,
            RECT_INSTANCE_SIZE,
            VertexInputRate::Instance,
            VertexFormat::Float32x4
        );
        rect_invocation->bind_vertex_attribute(     // instance uv
            5,
            *rect_instance_buffer,
            4 * 20,
            RECT_INSTANCE_SIZE,
            VertexInputRate::Instance,
            VertexFormat::Float32x4
        );
        rect_invocation->bind_vertex_attribute(     // instance info
            6,
            *rect_instance_buffer,
            4 * 24,
            RECT_INSTANCE_SIZE,
            VertexInputRate::Instance,
            VertexFormat::Float32x4
        );
        rect_invocation->validate();

        glm::mat4 projection = glm::ortho(
            0.0f, static_cast<float>(target.get_width()),
            static_cast<float>(target.get_height()), 0.0f,
            -1.0f, 1.0f
        );
        rect_invocation->set_uniform<glm::mat4>("camera.projection", projection);

        // misc settings
        // TODO set alpha blending mode
        rect_invocation->do_clear_depth(false);
        rect_invocation->do_depth_testing(false);
        rect_invocation->do_clear_color(false);
        rect_invocation->do_clipping(true);
    }

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
        blur_invocation_horizontal->validate();
        blur_invocation_horizontal->bind_sampled_texture({0, 1}, *compositing_texture, *default_sampler);
        blur_invocation_horizontal->set_uniform("settings.is_vertical", 0);
        blur_invocation_horizontal->do_clear_color(true);

        // blur texture -> compositing texture
        blur_invocation_vertical = blur_program->create_invocation();
        blur_invocation_vertical->bind_render_target(*compositing_target);
        blur_invocation_vertical->validate();
        blur_invocation_vertical->bind_sampled_texture({0, 1}, *blur_texture, *default_sampler);
        blur_invocation_vertical->set_uniform("settings.is_vertical", 1);
        blur_invocation_vertical->do_clear_color(false);
        blur_invocation_vertical->do_clipping(true);
    }
}

void UIRenderer::draw_rect(Bounds rect, glm::vec4 color) {
    rect.pos += get_origin();
    commands.push_back(DrawRect {rect, color});
}

void UIRenderer::draw_textured_rect(Bounds rect, Texture& texture) {
    rect.pos += get_origin();
    uint64_t texture_id = texture.get_texture_id();
    textures.insert({texture_id, &texture});
    commands.push_back(DrawTexturedRect {rect, {{0, 0}, {1, 1}}, texture_id, glm::vec4(1)});
}

void UIRenderer::draw_line(
    glm::vec2 start, 
    glm::vec2 end, 
    float width, 
    glm::vec4 color
) {
    start += get_origin();
    end += get_origin();
    commands.push_back(DrawLine {start, end, width, color});
}

void UIRenderer::draw_rect_outline(Bounds rect, glm::vec4 color, float inset) {
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
    textures.insert({texture_id, &atlas_texture});

    // rect commands
    glm::vec2 pen = start + get_origin();
    glm::vec2 atlas_size = {atlas.get_width(), atlas.get_height()};
    for(int i = 0; i < text.size(); i++) {
        char c = text[i];
        Font::Glyph& glyph = font.get_glyph(c);

        glm::vec2 pos = pen;
        pos.x += glyph.bearing_x;
        pos.y += glyph.bearing_y;
        pos = glm::round(pos);  // need to round to not let sampling mess with text sharpness
        glm::vec2 dim = {glyph.width, glyph.height};
        dim += glm::vec2 {0.5, 0.5};

        glm::vec2 uv_pos = {glyph.atlas_x, glyph.atlas_y};
        uv_pos.x /= atlas.get_width();
        uv_pos.y /= atlas.get_height();
        glm::vec2 uv_dim = dim;
        uv_dim.x /= atlas.get_width();
        uv_dim.y /= atlas.get_height();

        // constexpr float PAD = 1.0f;   // pixels
        // glm::vec2 pos = glm::round(pen + glm::vec2(glyph.bearing_x, glyph.bearing_y));
        // glm::vec2 pos = pen + glm::vec2(glyph.bearing_x, glyph.bearing_y) - glm::vec2(PAD);
        // glm::vec2 dim = glm::vec2(glyph.width, glyph.height) + glm::vec2(2 * PAD);

        // glm::vec2 uv_pos = (glm::vec2(glyph.atlas_x, glyph.atlas_y) - glm::vec2(PAD)) / atlas_size;
        // glm::vec2 uv_dim = dim / atlas_size;

        commands.push_back(DrawTexturedRect {
            {pos, dim}, {uv_pos, uv_dim},
            texture_id,
            color
        });
        
        pen.x += glyph.advance_x;
    }
}

void UIRenderer::draw_blur(Bounds rect, uint32_t sigma) {
    rect.pos += get_origin();
    commands.push_back(BlurRect {rect, sigma});
}

void UIRenderer::push_origin(glm::vec2 origin) {
    origin_stack.push(get_origin() + origin);
}

void UIRenderer::pop_origin() {
    assert(origin_stack.size() != 0);
    if(origin_stack.size() == 1) {
        engine::util::throw_error("cannot pop base origin");
    }
    origin_stack.pop();
}

glm::vec2 UIRenderer::get_origin() {
    assert(origin_stack.size() >= 1);
    return origin_stack.top();
}

void UIRenderer::push_clip(Bounds rect) {
    // new rect is relative to current origin
    rect.pos += get_origin();

    // intersect along both axes
    Bounds old_rect = get_clip();
    glm::vec2 min = glm::max(rect.pos, old_rect.pos);
    glm::vec2 max = glm::min(rect.pos + rect.dim, old_rect.pos + old_rect.dim);
    rect = Bounds {min, max - min};

    // check for degenerate clipping rectangle
    if(rect.dim.x <= 0 || rect.dim.y <= 0) {
        rect = Bounds {{0, 0}, {0, 0}};
    }
    
    clip_stack.push(rect);
    commands.push_back(SetClip {rect});
}

void UIRenderer::pop_clip() {
    assert(clip_stack.size() != 0);
    if(clip_stack.size() == 1) {
        engine::util::throw_error("cannot pop base clip rect");
    }
    clip_stack.pop();
    commands.push_back(SetClip {clip_stack.top()});
}

Bounds UIRenderer::get_clip() {
    assert(clip_stack.size() >= 1);
    return clip_stack.top();
}

void UIRenderer::render() {
    compositing_target->clear(engine::graphics::ClearFlags::Color);

    // buffer for uploading data to the rect instance buffer
    float* cpu_buf = (float*) malloc(RECT_INSTANCE_BUFFER_SIZE);
    uint64_t cpu_buf_ptr = 0;
    uint64_t cpu_instance_cnt = 0;
    uint64_t gpu_buf_ptr = 0;
    uint64_t gpu_instance_ptr = 0;
    std::optional<Texture*> bound_texture = std::nullopt;

    // current clip rect state
    int32_t clip_min_x = 0, clip_min_y = 0;
    uint32_t clip_width = 0, clip_height = 0;

    auto flush_instances = [&]() {
        if(cpu_instance_cnt == 0) return;

        // upload rect data to GPU buffer
        uint64_t cpu_buf_upload_sz = sizeof(float) * cpu_buf_ptr;
        assert(gpu_buf_ptr + cpu_buf_upload_sz <= RECT_INSTANCE_BUFFER_SIZE);
        rect_instance_buffer->upload_data(cpu_buf, cpu_buf_upload_sz, gpu_buf_ptr);

        // bind textures
        if(bound_texture.has_value()) {
            rect_invocation->bind_sampled_texture(
                {0, 1},
                *bound_texture.value(),
                *default_sampler
            );
        }
        else {
            rect_invocation->bind_sampled_texture(
                {0, 1},
                *default_texture,
                *default_sampler
            );
        }

        // set clip rect
        rect_invocation->set_clip_rect(
            clip_min_x, clip_min_y,
            clip_width, clip_height
        );

        // render
        rect_invocation->draw(2, cpu_instance_cnt, 0, gpu_instance_ptr);

        // reset command batch state
        gpu_buf_ptr += cpu_buf_upload_sz;
        gpu_instance_ptr += cpu_instance_cnt;
        bound_texture = std::nullopt;

        cpu_buf_ptr = 0;
        cpu_instance_cnt = 0;
    };
    auto write_instance = [&](
        glm::mat4 transform, 
        glm::vec4 color, 
        glm::vec4 uv = {0, 0, 1, 1},
        bool sample_texture = false
    ) {
        assert(cpu_instance_cnt < RECT_INSTANCE_AMT);
        cpu_instance_cnt ++;

        // transform
        const float* data = glm::value_ptr(transform);
        for(int i = 0; i < 16; i++) {
            cpu_buf[cpu_buf_ptr ++] = data[i];
        }
        
        // color
        cpu_buf[cpu_buf_ptr ++] = color.r * color.a;
        cpu_buf[cpu_buf_ptr ++] = color.g * color.a;
        cpu_buf[cpu_buf_ptr ++] = color.b * color.a;
        cpu_buf[cpu_buf_ptr ++] = color.a;

        // uv
        cpu_buf[cpu_buf_ptr ++] = uv.x;
        cpu_buf[cpu_buf_ptr ++] = uv.y;
        cpu_buf[cpu_buf_ptr ++] = uv.z;
        cpu_buf[cpu_buf_ptr ++] = uv.w;

        // info
        cpu_buf[cpu_buf_ptr ++] = sample_texture ? 1.0f : 0.0f;
        cpu_buf[cpu_buf_ptr ++] = 0;
        cpu_buf[cpu_buf_ptr ++] = 0;
        cpu_buf[cpu_buf_ptr ++] = 0;

        // flush if buffer is full
        if(cpu_instance_cnt == RECT_INSTANCE_AMT) {
            flush_instances();
        }
    };
    auto bind_texture = [&](uint64_t texture_id) {
        assert(textures.contains(texture_id));
        Texture* texture = textures.at(texture_id);
        if(bound_texture.has_value() && texture->get_texture_id() != bound_texture.value()->get_texture_id()) {
            flush_instances();
        }
        bound_texture = texture;
    };
    auto set_clip = [&](
        int32_t min_x, int32_t min_y,
        uint32_t width, uint32_t height
    ) { 
        // see if nothing changed
        if(clip_min_x == min_x && clip_min_y == min_y && clip_width == width && clip_height == height) {
            return;
        }

        // flush instances and set clip
        flush_instances();
        clip_min_x = min_x;
        clip_min_y = min_y;
        clip_width = width;
        clip_height = height;

        rect_invocation->set_clip_rect(min_x, min_y, width, height);
        blur_invocation_vertical->set_clip_rect(min_x, min_y, width, height);
    };

    // reset clip rect
    set_clip(0, 0, target.get_width(), target.get_height());

    // evaluate commands
    for(Command _cmd : commands) {
        if(std::holds_alternative<SetClip>(_cmd)) {
            SetClip cmd = std::get<SetClip>(_cmd);
            Bounds rect = cmd.rect;

            int32_t min_x = glm::round(rect.pos.x);
            int32_t min_y = glm::round(rect.pos.y);
            uint32_t width = glm::round(rect.dim.x);
            uint32_t height = glm::round(rect.dim.y);
            set_clip(min_x, min_y, width, height);
        }
        else if(std::holds_alternative<DrawRect>(_cmd)) {
            DrawRect cmd = std::get<DrawRect>(_cmd);
            Bounds rect = cmd.rect;
            glm::vec4 color = cmd.color;

            glm::mat4 transform(
                rect.dim.x,     0.0f,        0.0f, 0.0f,
                0.0f,           rect.dim.y,  0.0f, 0.0f,
                0.0f,           0.0f,        1.0f, 0.0f,
                rect.pos.x,     rect.pos.y,  0.0f, 1.0f
            );
            write_instance(transform, color);
        }
        else if(std::holds_alternative<DrawLine>(_cmd)) {
            DrawLine cmd = std::get<DrawLine>(_cmd);
            glm::vec2 start = cmd.start;
            glm::vec2 end = cmd.end;
            float width = cmd.width;
            glm::vec4 color = cmd.color;

            auto snap_center = [](float v, float w) -> float {
                return (static_cast<int>(w) % 2 == 1) ? std::floor(v) + 0.5f : std::round(v);
            };
            float dx = end.x - start.x;
            float dy = end.y - start.y;
            const float eps = 1e-4f;

            glm::mat4 transform(1.0f);
            if(std::abs(dy) < eps) {                       // horizontal
                float y = snap_center(start.y, width);
                float x0 = std::round(std::min(start.x, end.x));
                float x1 = std::round(std::max(start.x, end.x)) + 1.0;  // include last pixel
                transform = glm::translate(glm::mat4(1.0f), glm::vec3(x0, y - width / 2.0f, 0.0f));
                transform = glm::scale(transform, glm::vec3(x1 - x0, width, 1.0f));
            }
            else if(std::abs(dx) < eps) {                  // vertical
                float x = snap_center(start.x, width);
                float y0 = std::round(std::min(start.y, end.y));
                float y1 = std::round(std::max(start.y, end.y)) + 1.0;  // include last pixel
                transform = glm::translate(glm::mat4(1.0f), glm::vec3(x - width / 2.0f, y0, 0.0f));
                transform = glm::scale(transform, glm::vec3(width, y1 - y0, 1.0f));
            }
            else {                                          // diagonal
                glm::vec2 a{start.x + 0.5f, start.y + 0.5f};
                glm::vec2 b{end.x + 0.5f,   end.y + 0.5f};

                glm::vec2 d = b - a;
                float len = glm::length(d);
                if (len > 1e-6f) {
                    glm::vec2 dir = d / len;
                    a -= dir * 0.5f;      // extend half a pixel past each end
                    b += dir * 0.5f;
                }

                float length = glm::length(b - a);
                float angle  = std::atan2(b.y - a.y, b.x - a.x);
                transform = glm::translate(glm::mat4(1.0f), glm::vec3(a.x, a.y, 0.0f));
                transform = glm::rotate(transform, angle, glm::vec3(0, 0, 1));
                transform = glm::translate(transform, glm::vec3(0.0f, -width / 2.0f, 0.0f));
                transform = glm::scale(transform, glm::vec3(length, width, 1.0f));
            }

            write_instance(transform, color);
        }
        else if(std::holds_alternative<DrawTexturedRect>(_cmd)) {
            DrawTexturedRect cmd = std::get<DrawTexturedRect>(_cmd);
            Bounds vertex_rect = cmd.vertex_rect;
            Bounds uv_rect = cmd.uv_rect;
            uint64_t texture_id = cmd.texture_id;
            glm::vec4 color = cmd.color;

            // bind texture
            bind_texture(texture_id);

            // write instance
            glm::mat4 transform(
                vertex_rect.dim.x,  0.0f,               0.0f, 0.0f,
                0.0f,               vertex_rect.dim.y,  0.0f, 0.0f,
                0.0f,               0.0f,               1.0f, 0.0f,
                vertex_rect.pos.x,  vertex_rect.pos.y,  0.0f, 1.0f
            );
            write_instance(
                transform,
                color, 
                {uv_rect.pos, uv_rect.pos + uv_rect.dim},
                true
            );
        }
        else if(std::holds_alternative<BlurRect>(_cmd)) {
            BlurRect cmd = std::get<BlurRect>(_cmd);
            Bounds rect = cmd.rect;
            uint32_t sigma = cmd.sigma;

            // flush rects
            flush_instances();

            // do blur
            // right now, the blur isn't really correct since it gets cut off vertically
            // in order to fix, probably have to decide on blur kernel size and adjust rect bounds accordingly
            glm::vec2 target_size = {target.get_width(), target.get_height()};
            glm::vec2 uv_pos = rect.pos / target_size;
            glm::vec2 uv_dim = rect.dim / target_size;
            blur_invocation_horizontal->set_uniform<glm::vec2>("settings.pos", uv_pos);
            blur_invocation_horizontal->set_uniform<glm::vec2>("settings.dim", uv_dim);
            blur_invocation_horizontal->set_uniform<int>("settings.sigma", sigma);
            blur_invocation_vertical->set_uniform<glm::vec2>("settings.pos", uv_pos);
            blur_invocation_vertical->set_uniform<glm::vec2>("settings.dim", uv_dim);
            blur_invocation_vertical->set_uniform<int>("settings.sigma", sigma);
            blur_invocation_horizontal->draw(1);
            blur_invocation_vertical->draw(1);
        }
        else assert(false);
    }
    flush_instances();
    free(cpu_buf);

    // render compositing texture to target
    compositing_invocation->draw(1);

    reset();
}

void UIRenderer::reset() {
    commands.clear();
    while(origin_stack.size() != 0) {
        origin_stack.pop();
    }
    origin_stack.push({0, 0});
    while(clip_stack.size() != 0) {
        clip_stack.pop();
    }
    clip_stack.push(Bounds {{0, 0}, {target.get_width(), target.get_height()}});
}
    
}