#version 460 core

layout (location = 0) in vec4 packed_pos;   // xy, zw
layout (location = 1) in vec4 color;

layout (location = 0) out vec2 frag_pos;
layout (location = 1) out vec4 frag_color;

layout (binding = 0) uniform Camera {
    mat4 projection;
} camera;

void main() {
    vec2 pos = gl_VertexID == 0 ? packed_pos.xy : packed_pos.zw;
    gl_Position = camera.projection * vec4(pos, 0.0, 1.0);
    frag_pos = pos;
    frag_color = color;
}