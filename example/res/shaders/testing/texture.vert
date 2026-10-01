#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

layout (location = 0) out vec3 frag_normal;
layout (location = 1) out vec2 frag_uv;

layout (binding = 0) uniform Camera {
    mat4 projection;
    mat4 view;
} camera;

void main() {
    gl_Position = camera.projection * camera.view * vec4(pos, 1.0);
    frag_normal = normal;
    frag_uv = uv;
}