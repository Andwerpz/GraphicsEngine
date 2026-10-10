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