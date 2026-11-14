#version 460 core

layout (location = 0) in vec2 frag_pos;
layout (location = 1) in vec2 frag_uv;
layout (location = 2) in vec4 frag_color;   // premultiplied rgba

layout (location = 0) out vec4 color;

layout (binding = 1) uniform sampler2D albedo;

void main() {
    color = texture(albedo, frag_uv) * frag_color;
}