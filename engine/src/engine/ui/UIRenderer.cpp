#include <engine/ui/UIRenderer.h>
#include <engine/utils/error.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>

#include <filesystem>
namespace fs = std::filesystem;

#include <iostream>

namespace engine::ui {

const char* rect_vert_source = R"(
#version 460 core

layout (location = 0) in mat4 model;
layout (location = 4) in vec4 color;
layout (location = 5) in vec4 uv;   // packed min xy, max xy
layout (location = 6) in vec4 info; 

layout (location = 0) out vec2 frag_pos;
layout (location = 1) out vec2 frag_uv;
layout (location = 2) out vec4 frag_color;
layout (location = 3) out vec4 frag_info;

layout (binding = 0) uniform Camera {
    mat4 projection;
} camera;

vec2 vertices[6] = vec2[](
    vec2(0, 0),
    vec2(1, 0),
    vec2(1, 1),
    vec2(0, 0),
    vec2(1, 1),
    vec2(0, 1)
);

void main() {
    vec2 pos = vertices[gl_VertexID];
    gl_Position = camera.projection * model * vec4(pos, 0.0, 1.0);
    frag_pos = (model * vec4(pos, 0.0, 1.0)).xy;
    frag_uv = mix(uv.xy, uv.zw, pos);
    frag_color = color;
    frag_info = info;
}
)";

const char* rect_frag_source = R"(
#version 460 core

layout (location = 0) in vec2 frag_pos;
layout (location = 1) in vec2 frag_uv;
layout (location = 2) in vec4 frag_color;
layout (location = 3) in vec4 frag_info;    // {read from texture, unused, unused, unused}

layout (location = 0) out vec4 color;

layout (binding = 1) uniform sampler2D albedo;

void main() {
    if(frag_info.x != 0.0) {
        // sample texture
        vec4 samp = texture(albedo, frag_uv);
        vec3 base_color = samp.rgb * frag_color.rgb;
        float alpha = samp.a * frag_color.a;
        color = vec4(base_color, alpha);
    }
    else {
        // single color
        color = frag_color;
    }
}
)";

UIRenderer::UIRenderer(
    GraphicsDevice& device,
    RenderTarget& target
) : device(device), 
    target(target) 
{
    reset();

    // create programs needed to render
    using Program = engine::graphics::Program;
    using ProgramInvocation = engine::graphics::ProgramInvocation;
    using Shader = engine::graphics::Shader;
    using ShaderStage = engine::graphics::ShaderStage;
    using PrimitiveTopology = engine::graphics::PrimitiveTopology;
    using BufferUsage = engine::graphics::BufferUsage;
    using BufferFrequency = engine::graphics::BufferFrequency;
    using VertexInputRate = engine::graphics::VertexInputRate;
    using VertexFormat = engine::graphics::VertexFormat;
    instance_buffer = device.create_buffer(INSTANCE_BUFFER_SIZE, BufferUsage::Vertex, BufferFrequency::Dynamic);

    // rect
    {
        auto rect_vertex_shader = device.create_shader(rect_vert_source, ShaderStage::Vertex);
        auto rect_fragment_shader = device.create_shader(rect_frag_source, ShaderStage::Fragment);
        rect_program = device.create_rendering_program(
            *rect_vertex_shader, 
            *rect_fragment_shader, 
            PrimitiveTopology::TriangleList
        );

        rect_invocation = rect_program->create_invocation();
        rect_invocation->bind_render_target(target);
        rect_invocation->bind_vertex_attribute(     // instance transform
            0,
            *instance_buffer,
            0,
            INSTANCE_SIZE,
            VertexInputRate::Instance,
            VertexFormat::Float32x4x4
        );
        rect_invocation->bind_vertex_attribute(     // instance color
            4,
            *instance_buffer,
            4 * 16,
            INSTANCE_SIZE,
            VertexInputRate::Instance,
            VertexFormat::Float32x4
        );
        rect_invocation->bind_vertex_attribute(     // instance uv
            5,
            *instance_buffer,
            4 * 20,
            INSTANCE_SIZE,
            VertexInputRate::Instance,
            VertexFormat::Float32x4
        );
        rect_invocation->bind_vertex_attribute(     // instance info
            6,
            *instance_buffer,
            4 * 24,
            INSTANCE_SIZE,
            VertexInputRate::Instance,
            VertexFormat::Float32x4
        );
        rect_invocation->validate();

        glm::mat4 projection = glm::ortho(
            0.0f, static_cast<float>(target.get_width()),
            static_cast<float>(target.get_height()), 0.0f,
            -1.0f, 1.0f
        );
        auto projection_mat4_token = rect_invocation->get_uniform<glm::mat4>("camera.projection");
        rect_invocation->set_uniform(projection_mat4_token, projection);

        // order instances are specified within instance buffer is order they will be rendered
        // TODO set alpha blending mode
        rect_invocation->do_clear_depth(false);
        rect_invocation->do_depth_testing(false);
        rect_invocation->do_clear_color(false);
    }

    default_texture = device.create_texture(
        engine::graphics::TextureDimension::D2,
        engine::graphics::TextureFormat::RGBA32F,
        1, 1, 1, 1
    );
    default_sampler = device.create_sampler();
}

void UIRenderer::draw_rect(UIRect rect, glm::vec4 color) {
    rect.pos += get_origin();
    commands.push_back(DrawRect {rect, color});
}

void UIRenderer::draw_textured_rect(UIRect rect, Texture& texture) {
    rect.pos += get_origin();
    uint64_t texture_id = texture.get_texture_id();
    textures.insert({texture_id, &texture});
    commands.push_back(DrawTexturedRect {rect, {{0, 0}, {1, 1}}, texture_id});
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

void UIRenderer::draw_rect_outline(UIRect rect, glm::vec4 color, float inset) {
    rect.dim = rect.dim - glm::vec2(inset * 2.0f);
    glm::vec2 c0 = rect.pos;
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
    Font& font
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
    for(int i = 0; i < text.size(); i++) {
        char c = text[i];
        Font::Glyph& glyph = font.get_glyph(c);

        glm::vec2 pos = pen;
        pos.x += glyph.bearing_x;
        pos.y += glyph.bearing_y;
        glm::vec2 dim = {glyph.width, glyph.height};

        glm::vec2 uv_pos = {glyph.atlas_x, glyph.atlas_y};
        uv_pos.x /= atlas.get_width();
        uv_pos.y /= atlas.get_height();
        glm::vec2 uv_dim = dim;
        uv_dim.x /= atlas.get_width();
        uv_dim.y /= atlas.get_height();

        commands.push_back(DrawTexturedRect {
            {pos, dim}, {uv_pos, uv_dim},
            texture_id
        });
        
        pen.x += glyph.advance_x;
    }
}

void UIRenderer::push_origin(glm::vec2 origin) {
    origin_stack.push(get_origin() + origin);
}

void UIRenderer::pop_origin() {
    assert(origin_stack.size() != 0);
    if(origin_stack.size() == 1) {
        engine::utils::throw_error("cannot pop base origin");
    }
    origin_stack.pop();
}

glm::vec2 UIRenderer::get_origin() {
    assert(origin_stack.size() >= 1);
    return origin_stack.top();
}

void UIRenderer::push_clip(UIRect rect) {
    // new rect is relative to current origin
    rect.pos += get_origin();

    // intersect along both axes
    UIRect old_rect = get_clip();
    glm::vec2 min = glm::max(rect.pos, old_rect.pos);
    glm::vec2 max = glm::min(rect.pos + rect.dim, old_rect.pos + old_rect.dim);
    rect = UIRect {min, max - min};

    // check for degenerate clipping rectangle
    if(rect.dim.x <= 0 || rect.dim.y <= 0) {
        rect = UIRect {{0, 0}, {0, 0}};
    }
    
    clip_stack.push(rect);
    commands.push_back(SetClip {rect});
}

void UIRenderer::pop_clip() {
    assert(clip_stack.size() != 0);
    if(clip_stack.size() == 1) {
        engine::utils::throw_error("cannot pop base clip rect");
    }
    clip_stack.pop();
    commands.push_back(SetClip {clip_stack.top()});
}

UIRect UIRenderer::get_clip() {
    assert(clip_stack.size() >= 1);
    return clip_stack.top();
}

void UIRenderer::render() {
    // buffer for uploading data to the instance buffer
    assert(INSTANCE_BUFFER_SIZE % sizeof(float) == 0);
    const uint64_t INSTANCE_AMT = INSTANCE_BUFFER_SIZE / INSTANCE_SIZE;
    float* cpu_buf = (float*) malloc(INSTANCE_BUFFER_SIZE);
    uint64_t cpu_buf_ptr = 0;
    uint64_t cpu_instance_cnt = 0;

    uint64_t gpu_buf_ptr = 0;
    uint64_t gpu_instance_ptr = 0;
    std::optional<Texture*> bound_texture = std::nullopt;

    auto flush_instances = [&]() {
        if(cpu_instance_cnt == 0) return;

        // upload rect data to GPU buffer
        uint64_t cpu_buf_upload_sz = sizeof(float) * cpu_buf_ptr;
        assert(gpu_buf_ptr + cpu_buf_upload_sz <= INSTANCE_BUFFER_SIZE);
        instance_buffer->upload_data(cpu_buf, cpu_buf_upload_sz, gpu_buf_ptr);

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
        assert(cpu_instance_cnt < INSTANCE_AMT);
        cpu_instance_cnt ++;

        // transform
        const float* data = glm::value_ptr(transform);
        for(int i = 0; i < 16; i++) {
            cpu_buf[cpu_buf_ptr ++] = data[i];
        }
        
        // color
        cpu_buf[cpu_buf_ptr ++] = color.r;
        cpu_buf[cpu_buf_ptr ++] = color.g;
        cpu_buf[cpu_buf_ptr ++] = color.b;
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
        if(cpu_instance_cnt == INSTANCE_AMT) {
            flush_instances();
        }
    };

    for(Command _cmd : commands) {
        if(std::holds_alternative<SetClip>(_cmd)) {
            assert(false);
        }
        else if(std::holds_alternative<DrawRect>(_cmd)) {
            DrawRect cmd = std::get<DrawRect>(_cmd);
            UIRect rect = cmd.rect;
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
            UIRect vertex_rect = cmd.vertex_rect;
            UIRect uv_rect = cmd.uv_rect;
            uint64_t texture_id = cmd.texture_id;

            // bind texture
            assert(textures.contains(texture_id));
            Texture* texture = textures.at(texture_id);
            if(bound_texture.has_value() && texture->get_texture_id() != bound_texture.value()->get_texture_id()) {
                flush_instances();
            }
            bound_texture = texture;

            // write instance
            glm::mat4 transform(
                vertex_rect.dim.x,  0.0f,               0.0f, 0.0f,
                0.0f,               vertex_rect.dim.y,  0.0f, 0.0f,
                0.0f,               0.0f,               1.0f, 0.0f,
                vertex_rect.pos.x,  vertex_rect.pos.y,  0.0f, 1.0f
            );
            write_instance(
                transform,
                {1, 1, 1, 1}, 
                {uv_rect.pos, uv_rect.pos + uv_rect.dim},
                true
            );
        }
        else assert(false);
    }
    flush_instances();

    free(cpu_buf);
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
    clip_stack.push(UIRect {{0, 0}, {target.get_width(), target.get_height()}});
}
    
}