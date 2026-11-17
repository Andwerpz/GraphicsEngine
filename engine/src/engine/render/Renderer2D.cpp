#include <engine/render/Renderer2D.h>
#include <engine/util/error.h>
#include <engine/asset/EmbeddedResource.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

namespace engine::render {

Renderer2D::Renderer2D(
    GraphicsDevice& device,
    RenderTarget& target
) : device(device),
    target(target)
{
    reset();

    using namespace engine::graphics;
    uint32_t width = target.get_width();
    uint32_t height = target.get_height();

    {
        default_texture = device.create_texture(
            TextureDimension::D2,
            TextureFormat::RGBA32F,
            1, 1, 1, 1
        );
        float data[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        default_texture->upload_data(data);

        default_sampler = device.create_sampler(
            /* min_filter = */ SamplerFilter::Linear,
            /* mag_filter = */ SamplerFilter::Linear,
            /* mipmap_filter = */ SamplerMipmapFilter::None,
            /* address_u = */ SamplerAddressMode::ClampToEdge,
            /* address_v = */ SamplerAddressMode::ClampToEdge,
            /* address_w = */ SamplerAddressMode::ClampToEdge
        );
    }

    auto create_program = [&](const std::string& name) -> std::unique_ptr<RenderingProgram> {
        auto vertex_source = engine::asset::load_asset("engine://res/shaders/render/renderer2d/" + name + ".vert");
        auto fragment_source = engine::asset::load_asset("engine://res/shaders/render/renderer2d/" + name + ".frag");
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

    // compositing
    {
        compositing_texture = device.create_texture(
            TextureDimension::D2,
            TextureFormat::RGBA32F,
            width, height, 1, 1
        );
        compositing_target = device.create_render_target(width, height);
        compositing_target->bind_texture(0, *compositing_texture);
        compositing_target->validate();

        compositing_program = create_program("fullscreen");
        compositing_to_target = compositing_program->create_invocation();
        compositing_to_target->bind_render_target(target);
        compositing_to_target->bind_sampled_texture(
            {0, 0}, *compositing_texture, *default_sampler
        );
    }

    // quads
    {
        quad_instance_buffer = device.create_buffer(
            QUAD_BUFFER_SIZE,
            BufferUsage::Vertex,
            BufferFrequency::Dynamic
        );

        quad_program = create_program("quad");
        quad_drawer = quad_program->create_invocation();
        quad_drawer->bind_render_target(*compositing_target);
        quad_drawer->bind_vertex_attribute(     // instance model mat4
            0,
            *quad_instance_buffer,
            0,
            QUAD_INSTANCE_SIZE,
            VertexInputRate::Instance,
            VertexFormat::Float32x4x4
        );
        quad_drawer->bind_vertex_attribute(     // instance uv
            4,
            *quad_instance_buffer,
            4 * 16,
            QUAD_INSTANCE_SIZE,
            VertexInputRate::Instance,
            VertexFormat::Float32x4
        );
        quad_drawer->bind_vertex_attribute(     // instance color
            5,
            *quad_instance_buffer,
            4 * 20,
            QUAD_INSTANCE_SIZE,
            VertexInputRate::Instance,
            VertexFormat::Float32x4
        );

        glm::mat4 projection = glm::ortho(
            0.0f, static_cast<float>(width),
            static_cast<float>(height), 0.0f,
            -1.0f, 1.0f
        );
        quad_drawer->set_uniform<glm::mat4>("camera.projection", projection);

        // TODO set alpha blending mode
        quad_drawer->do_clear_depth(false);
        quad_drawer->do_depth_testing(false);
        quad_drawer->do_clear_color(false);
        quad_drawer->do_clipping(true);
    }

    // scratch
    {
        scratch_texture = device.create_texture(
            TextureDimension::D2,
            TextureFormat::RGBA32F,
            width, height, 1, 1
        );
        scratch_target = device.create_render_target(width, height);
        scratch_target->bind_texture(0, *scratch_texture);
        scratch_target->validate();

        scratch_program = create_program("fullscreen");
        scratch_to_compositing = compositing_program->create_invocation();
        scratch_to_compositing->bind_render_target(*compositing_target);
        scratch_to_compositing->bind_sampled_texture(
            {0, 0}, *scratch_texture, *default_sampler
        );
        scratch_to_compositing->do_clear_color(false);
        scratch_to_compositing->do_clear_depth(false);
    }
}

void Renderer2D::draw_quad(glm::mat4 model, glm::vec4 color) {
    push_quad(model, Rect2 {{0, 0}, {1, 1}}, color, Material {nullptr});
}

void Renderer2D::draw_quad(glm::mat4 model, Texture& texture, glm::vec4 color, Rect2 uv) {
    push_quad(model, uv, color, Material {&texture});
}

void Renderer2D::draw_rect(Rect2 rect, glm::vec4 color) {
    push_rect(rect, Rect2 {{0, 0}, {1, 1}}, color, Material {nullptr});
}

void Renderer2D::draw_rect(Rect2 rect, Texture& texture, glm::vec4 color, Rect2 uv) {
    push_rect(rect, uv, color, Material {&texture});
}

void Renderer2D::draw_custom_pass(CustomPass pass) {
    commands.push_back(CustomPass {std::move(pass)});
}

void Renderer2D::push_origin(glm::vec2 origin) {
    origin_stack.push(get_origin() + origin);
}

void Renderer2D::pop_origin() {
    assert(origin_stack.size() != 0);
    if(origin_stack.size() == 1) {
        engine::util::throw_error("cannot pop base origin");
    }
    origin_stack.pop();
}

glm::vec2 Renderer2D::get_origin() {
    assert(origin_stack.size() >= 1);
    return origin_stack.top();
}

void Renderer2D::push_clip(Rect2 rect) {
    // new rect is relative to current origin
    rect.pos += get_origin();

    // intersect to get new clip
    Rect2 new_rect = clip_stack.top().intersect(rect);

    clip_stack.push(new_rect);
    commands.push_back(SetClip {new_rect});
}

void Renderer2D::pop_clip() {
    assert(clip_stack.size() != 0);
    if(clip_stack.size() == 1) {
        engine::util::throw_error("cannot pop base clip rect");
    }
    clip_stack.pop();
    commands.push_back(SetClip {clip_stack.top()});
}

void Renderer2D::render() {
    compositing_target->clear(engine::graphics::ClearFlags::Color);

    // current clip state, in pixels
    int32_t clip_min_x = 0, clip_min_y = 0;
    uint32_t clip_width = target.get_width();
    uint32_t clip_height = target.get_height();

    // go through all commands
    for(const Command& _cmd : commands) {
        if(const SetClip* cmd = std::get_if<SetClip>(&_cmd)) {
            const Rect2& rect = cmd->clip_rect;
            clip_min_x = static_cast<int32_t>(glm::round(rect.pos.x));
            clip_min_y = static_cast<int32_t>(glm::round(rect.pos.y));
            clip_width = static_cast<uint32_t>(std::max(0.0f, glm::round(rect.dim.x)));
            clip_height = static_cast<uint32_t>(std::max(0.0f, glm::round(rect.dim.y)));
        }
        else if(const DrawQuadBatch* cmd = std::get_if<DrawQuadBatch>(&_cmd)) {
            Texture& texture = cmd->material.texture ? *cmd->material.texture : *default_texture;
            quad_drawer->bind_sampled_texture({0, 1}, texture, *default_sampler);
            quad_drawer->set_clip_rect(clip_min_x, clip_min_y, clip_width, clip_height);

            // draw batches in GPU buffer sized chunks
            uint64_t done_amt = 0;
            while(done_amt < cmd->amt) {
                uint64_t n = std::min<uint64_t>(cmd->amt - done_amt, QUAD_INSTANCE_CAPACITY);
                quad_instance_buffer->upload_data(
                    quad_instances.data() + cmd->start + done_amt,
                    n * QUAD_INSTANCE_SIZE
                );
                quad_drawer->draw(2, n);
                done_amt += n;
            }
        }
        else if(const CustomPass* cmd = std::get_if<CustomPass>(&_cmd)) {
            // create and clear view into scratch target
            auto& scratch_view = scratch_target;
            scratch_view->clear(engine::graphics::ClearFlags::Color);

            // custom pass renders to scratch view
            (*cmd)(*compositing_texture, *scratch_view);

            // render scratch back to compositing
            scratch_to_compositing->set_clip_rect(clip_min_x, clip_min_y, clip_width, clip_height);
            scratch_to_compositing->draw(1);
        }
        else assert(false);
    }

    // render compositing texture to target
    compositing_to_target->draw(1);

    reset();
}

void Renderer2D::reset() {
    commands.clear();
    quad_instances.clear();
    while(origin_stack.size() != 0) {
        origin_stack.pop();
    }
    origin_stack.push({0, 0});
    while(clip_stack.size() != 0) {
        clip_stack.pop();
    }
    clip_stack.push(Rect2 {{0, 0}, {target.get_width(), target.get_height()}});
}

void Renderer2D::emit_quad(const QuadInstance& quad, const Material& material) {
    // insert instance
    size_t index = quad_instances.size();
    quad_instances.push_back(quad);

    // try to batch with the previous command
    if(!commands.empty()) {
        if(DrawQuadBatch* batch = std::get_if<DrawQuadBatch>(&commands.back())) {
            assert(batch->start + batch->amt + 1 == quad_instances.size());
            if(batch->material.texture == material.texture && batch->start + batch->amt == index) {
                batch->amt++;
                return;
            }
        }
    }

    // failed to batch, create a new command
    commands.push_back(DrawQuadBatch {index, 1, material});
}

void Renderer2D::push_quad(glm::mat4 model, Rect2 uv, glm::vec4 color, Material material) {
    // adjust model origin
    model[3][0] += get_origin().x;
    model[3][1] += get_origin().y;
    
    // premultiply alpha
    color = glm::vec4(glm::vec3(color) * color.a, color.a);

    emit_quad(
        QuadInstance {
            .model = model,
            .uv = uv,
            .color = color
        },
        material
    );
}

void Renderer2D::push_rect(Rect2 rect, Rect2 uv, glm::vec4 color, Material material) {
    // snap to pixel
    // a pixel is drawn over if their center is within the rect
    float x0 = std::ceil(rect.pos.x - 0.5f);
    float y0 = std::ceil(rect.pos.y - 0.5f);
    float x1 = std::ceil(rect.max().x - 0.5f);
    float y1 = std::ceil(rect.max().y - 0.5f);

    // check for empty rectangle
    if(x1 <= x0 || y1 <= y0) {
        return;
    }

    glm::mat4 model(1.0f);
    model[0][0] = x1 - x0;
    model[1][1] = y1 - y0;
    model[3][0] = x0;
    model[3][1] = y0;
    push_quad(model, uv, color, material);
}

}