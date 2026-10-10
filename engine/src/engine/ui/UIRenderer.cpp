#include <engine/ui/UIRenderer.h>
#include <engine/utils/error.h>

#include <engine/graphics/Program.h>
#include <engine/graphics/ProgramInvocation.h>
#include <engine/graphics/Buffer.h>
#include <engine/graphics/Shader.h>

#include <engine/graphics/opengl/GLBuffer.h>

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

layout (location = 0) out vec2 frag_pos;
layout (location = 1) out vec2 frag_uv;
layout (location = 2) out vec4 frag_color;

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
    frag_uv = pos;
    frag_color = color;
}
)";

const char* rect_frag_source = R"(
#version 460 core

layout (location = 0) in vec2 frag_pos;
layout (location = 1) in vec2 frag_uv;
layout (location = 2) in vec4 frag_color;

layout (location = 0) out vec4 color;

void main() {
    // color = rect.color;
    // color = vec4(frag_uv, 0.0, 1.0);
    // color = vec4(1.0);
    color = frag_color;
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
            4 * 20,
            VertexInputRate::Instance,
            VertexFormat::Float32x4x4
        );
        rect_invocation->bind_vertex_attribute(     // instance color
            4,
            *instance_buffer,
            4 * 16,
            4 * 20,
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
}

void UIRenderer::draw_rect(Rect rect, Color color) {
    commands.push_back(DrawRect {rect, color});
}

void UIRenderer::draw_line(Vec2 start, Vec2 end, float width, Color color) {
    commands.push_back(DrawLine {start, end, width, color});
}

void UIRenderer::push_clip(Rect rect) {
    assert(clip_stack.size() >= 1);
    Rect old_rect = clip_stack.top();
    
    // intersect along both axes
    float xmin = std::max(rect.x, old_rect.x);
    float xmax = std::min(rect.x + rect.width, old_rect.x + old_rect.width);
    float ymin = std::max(rect.y, old_rect.y);
    float ymax = std::min(rect.y + rect.height, old_rect.y + old_rect.height);
    rect = Rect {xmin, ymin, xmax - xmin, ymax - ymin};

    // check for degenerate clipping rectangle
    if(rect.width <= 0 || rect.height <= 0) {
        rect = Rect {0, 0, 0, 0};
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

void UIRenderer::render() {
    // buffer for uploading data to the instance buffer
    assert(INSTANCE_BUFFER_SIZE % sizeof(float) == 0);
    const uint64_t INSTANCE_SIZE = sizeof(float) * 20;
    const uint64_t INSTANCE_AMT = INSTANCE_BUFFER_SIZE / INSTANCE_SIZE;
    float* cpu_buf = (float*) malloc(INSTANCE_BUFFER_SIZE);
    uint64_t cpu_buf_ptr = 0;
    uint64_t cpu_instance_cnt = 0;

    uint64_t gpu_buf_ptr = 0;
    uint64_t gpu_instance_ptr = 0;

    auto flush_instances = [&]() {
        if(cpu_instance_cnt == 0) return;

        // upload rect data to GPU buffer
        uint64_t cpu_buf_upload_sz = sizeof(float) * cpu_buf_ptr;
        assert(gpu_buf_ptr + cpu_buf_upload_sz <= INSTANCE_BUFFER_SIZE);
        instance_buffer->upload_data(cpu_buf, cpu_buf_upload_sz, gpu_buf_ptr);

        // render
        rect_invocation->draw(2, cpu_instance_cnt, 0, gpu_instance_ptr);

        // reset CPU buffer
        gpu_buf_ptr += cpu_buf_upload_sz;
        gpu_instance_ptr += cpu_instance_cnt;

        cpu_buf_ptr = 0;
        cpu_instance_cnt = 0;
    };
    auto write_instance = [&](glm::mat4 transform, Color color) {
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
            Rect rect = cmd.rect;
            Color color = cmd.color;

            glm::mat4 transform(
                rect.width,     0.0f,        0.0f, 0.0f,
                0.0f,           rect.height, 0.0f, 0.0f,
                0.0f,           0.0f,        1.0f, 0.0f,
                rect.x,         rect.y,      0.0f, 1.0f
            );
            write_instance(transform, color);
        }
        else if(std::holds_alternative<DrawLine>(_cmd)) {
            DrawLine cmd = std::get<DrawLine>(_cmd);
            Vec2 start = cmd.start;
            Vec2 end = cmd.end;
            float width = cmd.width;
            Color color = cmd.color;

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
        else assert(false);
    }
    flush_instances();

    free(cpu_buf);
    reset();
}

void UIRenderer::reset() {
    commands.clear();
    while(clip_stack.size() != 0) {
        clip_stack.pop();
    }
    clip_stack.push(Rect {0, 0, target.get_width(), target.get_height()});
}
    
}