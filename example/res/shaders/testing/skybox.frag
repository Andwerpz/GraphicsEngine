#version 460 core

layout (location = 0) in vec3 frag_pos;
layout (location = 1) in vec3 frag_normal;
layout (location = 2) in vec2 frag_uv;

layout (location = 0) out vec4 color;

layout (binding = 1) uniform samplerCube cubemap;

void main() {
    vec3 frag_dir = normalize(frag_pos);
    color = vec4(texture(cubemap, frag_dir).xyz, 1.0);
    // color = vec4(frag_dir, 1.0);
    // color = vec4(frag_normal * 0.5 + 0.5, 1.0);
}